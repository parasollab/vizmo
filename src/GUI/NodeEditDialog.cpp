#include "NodeEditDialog.h"

#include <sstream>

#include "GLWidget.h"
#include "MainWindow.h"
#include "ModelSelectionWidget.h"

#include "Models/ActiveMultiBodyModel.h"
#include "Models/BoundingBoxModel.h"
#include "Models/BoundingSphereModel.h"
#include "Models/CfgModel.h"
#include "Models/EnvModel.h"
#include "Models/MultiBodyModel.h"
#include "Models/QueryModel.h"
#include "Models/Vizmo.h"


/*-------------------------- Node Edit Validator -----------------------------*/

QValidator::State
NodeEditValidator::
validate(QString& _s, int& _i) const {
  if(_s.isEmpty() || _s == "-" || _s == "." || _s == "-.")
    return QValidator::Intermediate;

  bool ok;
  double d = _s.toDouble(&ok);

  if(ok && d >= m_min && d <= m_max)
    return QValidator::Acceptable;
  else
    return QValidator::Invalid;
}

/*---------------------------- Node Edit Slider ------------------------------*/

NodeEditSlider::
NodeEditSlider(QWidget* _parent, string _label) : QWidget(_parent) {
  setStyleSheet("QLabel { font:11pt } QLineEdit { font:11pt }");
  setMaximumHeight(60);

  QGridLayout* layout = new QGridLayout();
  this->setLayout(layout);

  QLabel* dofName = new QLabel(this);
  dofName->setText(QString::fromStdString(_label));
  layout->addWidget(dofName, 0, 0, 1, 14);

  m_slider = new QSlider(this);
  m_slider->setOrientation(Qt::Horizontal);
  m_slider->installEventFilter(this);
  layout->addWidget(m_slider, 2, 0, 1, 10);

  m_dofValue = new QLineEdit(this);
  layout->addWidget(m_dofValue, 2, 10, 1, 4);

  connect(m_slider, SIGNAL(valueChanged(int)), this, SLOT(UpdateDOFLabel(int)));
}


void
NodeEditSlider::
UpdateDOFLabel(int _newVal) {
  ostringstream oss;
  oss << (double)_newVal/100000.0;
  QString qs(oss.str().c_str());
  m_dofValue->setCursorPosition(0);
  m_dofValue->setText(qs);
}


void
NodeEditSlider::
MoveSlider(QString _inputVal) {
 m_slider->setSliderPosition(_inputVal.toDouble() * 100000.0);
}


bool
NodeEditSlider::
eventFilter(QObject* _target, QEvent* _event) {
  if(_target == m_slider && _event->type() == QEvent::Wheel) {
    _event->ignore(); //Prevent mouse wheel from moving sliders
    return true;
  }
  return false;
}

/*---------------------------- Node Edit Dialog ------------------------------*/

NodeEditDialog::
NodeEditDialog(MainWindow* _mainWindow, string _title, CfgModel* _originalNode,
    EdgeModel* _parentEdge) : QDialog(_mainWindow), m_title(_title),
    m_tempNode(NULL), m_originalNode(_originalNode),
    m_nodesToConnect(), m_nodesToDelete(), m_tempObjs() {
  //Set temporary objects
  if(_parentEdge == NULL) {
    //No parent edge, this is not an intermediate cfg.
    //Check all adjacent edges during modification.

    //Set temporary cfg for editing
    m_tempNode = new CfgModel(*_originalNode);
    m_tempObjs.AddModel(m_tempNode);

    //Set temporary edges
    if(GetVizmo().GetMap()) {
      Graph* graph = GetVizmo().GetMap()->GetGraph();
      VI vit = graph->find_vertex(m_originalNode->GetIndex());
      if(vit != graph->end()) {
        for(EI eit = vit->begin(); eit != vit->end(); ++eit) {
          m_nodesToConnect.push_back((*eit).target());
          EdgeModel* tempEdge = new EdgeModel((*eit).property());
          CfgModel* targetCfg = &(graph->find_vertex((*eit).target())->property());
          tempEdge->Set(m_tempNode, targetCfg);
          m_tempObjs.AddModel(tempEdge);
        }
      }
    }
  }
  else {
    //Parent edge specified, this is an intermediate cfg.
    //Check edges leading to and from this cfg
    for(vector<CfgModel>::iterator iit = _parentEdge->GetIntermediates().begin();
        iit != _parentEdge->GetIntermediates().end(); ++iit) {
      if(*iit == *_originalNode) {
        m_tempNode = new CfgModel(*iit);
        CfgModel* start;
        CfgModel* end;
        EdgeModel* startEdge = new EdgeModel();
        EdgeModel* endEdge = new EdgeModel();
        if(iit == _parentEdge->GetIntermediates().begin())
          start = _parentEdge->GetStartCfg();
        else
          start = &(*(iit - 1));
        if(iit == _parentEdge->GetIntermediates().end() - 1)
          end = _parentEdge->GetEndCfg();
        else
          end = &(*(iit + 1));
        startEdge->Set(start, m_tempNode);
        endEdge->Set(m_tempNode, end);
        m_tempObjs.AddModel(m_tempNode);
        m_tempObjs.AddModel(startEdge);
        m_tempObjs.AddModel(endEdge);
      }
    }
  }

  //Set up dialog widget
  Init();

  //Configure end behavior
  connect(this, SIGNAL(finished(int)), this, SLOT(FinalizeNodeEdit(int)));
}

//Edge Collapse Constructor
NodeEditDialog::
NodeEditDialog(MainWindow* _mainWindow, string _title,
    EdgeModel* _edge, CfgModel* _newNode) : QDialog(_mainWindow), m_title(_title),
    m_tempNode(NULL), m_originalNode(_newNode),
    m_nodesToConnect(), m_nodesToDelete(), m_tempObjs() {
    m_tempObjs.AddModel(_edge->GetStartCfg());
    m_tempObjs.AddModel(_edge->GetEndCfg());
}

//Add new node constructor
NodeEditDialog::
NodeEditDialog(MainWindow* _mainWindow, string _title)
    : QDialog(_mainWindow), m_title(_title),
    m_tempNode(NULL), m_originalNode(NULL),
    m_nodesToConnect(), m_nodesToDelete(), m_tempObjs() {
  //Set temporary cfg for editing
  m_tempNode = new CfgModel();
  m_tempObjs.AddModel(m_tempNode);

  //Set up dialog widget
  Init();

  //Configure end behavior
  connect(this, SIGNAL(finished(int)), this, SLOT(FinalizeNodeAdd(int)));
}


//Merge nodes constructor
NodeEditDialog::
NodeEditDialog(MainWindow* _mainWindow, string _title, CfgModel* _tempNode,
    vector<VID> _toConnect, vector<VID> _toDelete)
    : QDialog(_mainWindow), m_title(_title),
    m_tempNode(_tempNode), m_originalNode(NULL),
    m_nodesToConnect(_toConnect), m_nodesToDelete(_toDelete), m_tempObjs() {
  //Set temporary cfg for editing
  m_tempObjs.AddModel(m_tempNode);

  //Set temporary edges
  Graph* graph = GetVizmo().GetMap()->GetGraph();
  for(vector<VID>::iterator vit = m_nodesToConnect.begin();
      vit != m_nodesToConnect.end(); ++vit) {
    EdgeModel* tempEdge = new EdgeModel();
    CfgModel* targetCfg = &(graph->find_vertex(*vit)->property());
    tempEdge->Set(m_tempNode, targetCfg);
    m_tempObjs.AddModel(tempEdge);
  }

  //Set up dialog Widget
  Init();

  //Configure end behavior
  connect(this, SIGNAL(finished(int)), this, SLOT(FinalizeNodeMerge(int)));
}


void
NodeEditDialog::
Init() {
  setWindowTitle("Modify Node");
  setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Maximum);

  QLabel* nodeLabel = new QLabel(this);
  QScrollArea* scrollArea = new QScrollArea(this);
  scrollArea->setWidgetResizable(true);

  QVBoxLayout* scrollAreaBoxLayout = new QVBoxLayout();
  scrollAreaBoxLayout->setSpacing(4);
  scrollAreaBoxLayout->setContentsMargins(3, 7, 3, 7); //L, T, R, B

  QGroupBox* scrollAreaBox = new QGroupBox(this);
  scrollAreaBox->setLayout(scrollAreaBoxLayout);

  QPushButton* exportButton = new QPushButton("E&xport", this);
  connect(exportButton, SIGNAL(clicked()), this, SLOT(DumpRobotInfo()));

  QDialogButtonBox* okayCancel = new QDialogButtonBox(this);
  okayCancel->setOrientation(Qt::Horizontal);
  okayCancel->setStandardButtons(QDialogButtonBox::Cancel | QDialogButtonBox::Ok);
  connect(okayCancel, SIGNAL(accepted()), this, SLOT(accept()));
  connect(okayCancel, SIGNAL(rejected()), this, SLOT(reject()));

  QVBoxLayout* overallLayout = new QVBoxLayout();
  overallLayout->addWidget(nodeLabel);
  overallLayout->addWidget(scrollArea);
  overallLayout->addWidget(exportButton);
  overallLayout->addWidget(okayCancel);

  SetUpSliders(m_sliders);
  for(SIT it = m_sliders.begin(); it != m_sliders.end(); it++)
    scrollAreaBoxLayout->addWidget(*it, 1, Qt::AlignTop);

  scrollAreaBox->setMaximumHeight(m_sliders.size() * 100);

  scrollArea->setWidget(scrollAreaBox);
  this->setLayout(overallLayout);

  nodeLabel->setText(QString::fromStdString("<b>DOFs</b> of " + m_title + ":"));
  const vector<double>& currCfg = m_tempNode->GetData();
  InitSliderValues(currCfg);

  setAttribute(Qt::WA_DeleteOnClose);
}


void
NodeEditDialog::
SetUpSliders(vector<NodeEditSlider*>& _sliders) {
  if(m_title == "Collapse Edge")  {
    const vector<ActiveMultiBody::DOFInfo>& dofInfo =
      GetVizmo().GetEnv()->GetRobot(m_tempNode->GetRobotIndex())->GetDOFInfo();
    QSignalMapper* sliderMapper = new QSignalMapper(this);
    connect(sliderMapper, SIGNAL(mapped(int)), this, SLOT(UpdateDOF(int)));

    NodeEditSlider* s = new NodeEditSlider(this, "Position");
    QSlider* actualSlider = s->GetSlider();
    QLineEdit* dofValue = s->GetDOFValue();


    double minVal = dofInfo[0].m_minVal;
    double maxVal = dofInfo[0].m_maxVal;
    actualSlider->setRange(minVal, maxVal);
    dofValue->setValidator(new NodeEditValidator(minVal, maxVal, 5, dofValue));

    connect(actualSlider, SIGNAL(valueChanged(int)), sliderMapper, SLOT(map()));
    connect(dofValue, SIGNAL(textEdited(const QString&)),
        s, SLOT(MoveSlider(QString)));

    sliderMapper->setMapping(actualSlider, 0);
    _sliders.push_back(s);
  }
  else  {
    const vector<ActiveMultiBody::DOFInfo>& dofInfo =
      GetVizmo().GetEnv()->GetRobot(m_tempNode->GetRobotIndex())->GetDOFInfo();
  QSignalMapper* sliderMapper = new QSignalMapper(this);
  connect(sliderMapper, SIGNAL(mapped(int)), this, SLOT(UpdateDOF(int)));

  for(size_t i = 0; i < dofInfo.size(); i++) {
    NodeEditSlider* s = new NodeEditSlider(this, dofInfo[i].m_name);
    QSlider* actualSlider = s->GetSlider();
    QLineEdit* dofValue = s->GetDOFValue();

    double minVal = dofInfo[i].m_minVal;
    double maxVal = dofInfo[i].m_maxVal;
    actualSlider->setRange(100000*minVal, 100000*maxVal);
    dofValue->setValidator(new NodeEditValidator(minVal, maxVal, 5, dofValue));

    connect(actualSlider, SIGNAL(valueChanged(int)), sliderMapper, SLOT(map()));
    connect(dofValue, SIGNAL(textEdited(const QString&)),
        s, SLOT(MoveSlider(QString)));

    sliderMapper->setMapping(actualSlider, i);
    _sliders.push_back(s);
  }
  }
}


void
NodeEditDialog::
InitSliderValues(const vector<double>& _vals) {
  for(size_t i = 0; i < m_sliders.size(); i++) {
    (m_sliders[i])->GetSlider()->setSliderPosition(100000*_vals[i]);
    ostringstream oss;
    oss << _vals[i];
    QString qValLabel = QString::fromStdString(oss.str());
    (m_sliders[i])->GetDOFValue()->setText(qValLabel);
  }
}


void
NodeEditDialog::
UpdateDOF(int _id) {
  if(m_title=="Collapse Edge"){
    auto temp = m_tempObjs.GetModels();
  	auto s= *(static_cast<CfgModel*>(temp[0]));
  	auto t= *(static_cast<CfgModel*>(temp[1]));
		size_t num = GetVizmo().GetEnv()->GetRobot(m_tempNode->GetRobotIndex())->GetDOFInfo().size();
		for(size_t i = 0; i < num; ++i)
   		( *m_tempNode)[i] = s[i] + (m_sliders[0]->GetSlider()->value()/100000.0) * (t[i] - s[i]);

  }
  //Also assumes index alignment
  (*m_tempNode)[_id] = m_sliders[_id]->GetSlider()->value() / 100000.0;

  ValidityCheck();

  if(m_tempNode->IsQuery()) {
    GetVizmo().GetQry()->Build();
    GetVizmo().PlaceRobots();
  }
}


void
NodeEditDialog::
ValidityCheck() {
  // Check temp node.
  if(GetVizmo().GetEnv()->GetRobot(m_tempNode->GetRobotIndex())->
      InCSpace(m_tempNode->GetData())) {
    m_tempNode->SetValidity(true);
    GetVizmo().CollisionCheck(*m_tempNode);
  }

  // Check edges.
  for(auto m : m_tempObjs) {
    // Skip non-edge models.
    if(m->Name().substr(0, 4) != "Edge")
      continue;
    auto edge = static_cast<EdgeModel*>(m);
    edge->SetValidity(true);
    edge->SetWeight(0);

    // Define function for validating an edge.
    auto validate = [&edge](CfgModel& _c1, CfgModel& _c2) {
      pair<bool, double> visibility = GetVizmo().VisibilityCheck(_c1, _c2);
      edge->SetValidity(edge->IsValid() && visibility.first);
      edge->SetWeight(edge->GetWeight() + visibility.second);
    };

    // Validate intermediate edges.
    vector<CfgModel>& intermediates = edge->GetIntermediates();
    if(intermediates.empty())
      validate(*edge->GetStartCfg(), *edge->GetEndCfg());
    else {
      validate(*edge->GetStartCfg(), intermediates.front());
      for(auto i = intermediates.begin() + 1; i != intermediates.end(); ++i)
        validate(*(i - 1), *i);
      validate(intermediates.back(), *edge->GetEndCfg());
    }
  }
}


void
NodeEditDialog::
FinalizeNodeEdit(int _accepted) {
  Map* map = GetVizmo().GetMap();

  if(_accepted == 1) {  //user pressed okay
    if(m_tempNode->IsValid() || m_title.find("Vertex") != string::npos) {
      //set data for original node to match temp
      m_originalNode->SetCfg(m_tempNode->GetDataCfg());
      if(map && m_title.find("Vertex") == string::npos)  {
      //delete edges that are no longer valid
        Graph* graph = map->GetGraph();
        for(auto& m : m_tempObjs) {
          // Skip non-edges.
          if(m->Name().substr(0, 4) != "Edge")
            continue;
          auto edge = static_cast<EdgeModel*>(m);

          if(!edge->IsValid()) {
            VID start = map->Cfg2VID(*(edge->GetStartCfg()));
            VID end = map->Cfg2VID(*(edge->GetEndCfg()));
            graph->delete_edge(start, end);
            graph->delete_edge(end, start);
          }
        }
      }
    }
    else
      GetMainWindow()->AlertUser("Invalid configuration!");

    if(map)
      map->RefreshMap();

    if(GetVizmo().GetQry()) {
      GetVizmo().GetQry()->Build();
      GetVizmo().PlaceRobots();
    }
  }
  GetMainWindow()->GetModelSelectionWidget()->ResetLists();
}


void
NodeEditDialog::
FinalizeNodeAdd(int _accepted) {
  if(m_title.find("Vertex") != string::npos)	{
    if(_accepted == 1)	{
      Point3d p = m_tempNode->GetPoint();
      auto graph = GetVizmo().GetEnv()->GetGraphModel();

      if(!graph)	{
				GetVizmo().GetEnv()->AddEmptyGraphModel();
				graph = GetVizmo().GetEnv()->GetGraphModel();
      }

      graph->AddVertex(p);
      graph->Refresh();
      GetMainWindow()->GetModelSelectionWidget()->ResetLists();
    }
  }
  else	{
    Map* map = GetVizmo().GetMap();
    if(map) {
      Graph* graph = map->GetGraph();
      if(_accepted == 1) {
	if(m_tempNode->IsValid()) {
  	  CfgModel newNode = *m_tempNode;
	  newNode.SetRenderMode(SOLID_MODE);
	  graph->add_vertex(newNode);
	  map->RefreshMap();
	}
      	else
          QMessageBox::about(this, "", "Cannot add invalid node!");
	}
    }
    GetMainWindow()->GetModelSelectionWidget()->ResetLists();
  }
}


void
NodeEditDialog::
FinalizeNodeMerge(int _accepted) {
  Map* map = GetVizmo().GetMap();
  Graph* graph = map->GetGraph();

  if(_accepted == 1) {
    if(m_tempNode->IsValid()) {
      CfgModel super = *m_tempNode;
      super.SetRenderMode(SOLID_MODE);
      Map::VID superID = graph->add_vertex(super);

      //Add the valid new edges
      for(auto m : m_tempObjs) {
        // Skip non-edges.
        if(m->Name().substr(0, 4) != "Edge")
          continue;
        auto edge = static_cast<EdgeModel*>(m);

        if(edge->IsValid()) {
          graph->add_edge(superID, map->Cfg2VID(*edge->GetEndCfg()));
          graph->add_edge(map->Cfg2VID(*edge->GetEndCfg()), superID);
        }
      }

      //Remove selected vertices
      for(const auto vid : m_nodesToDelete)
        graph->delete_vertex(vid);

      map->RefreshMap();
    }
    else
      QMessageBox::about(this, "", "Invalid merge!");
  }
  GetMainWindow()->GetModelSelectionWidget()->ResetLists();
}

void
NodeEditDialog::
DumpRobotInfo() {
  auto robot = m_tempNode->GetRobot();
  cout << "Robot info (as in env file):\n\n";
  robot->Write(std::cout);

  cout << "\nBody transforms:";
  for(size_t i = 0; i < robot->NumFreeBody(); ++i)
    cout << "\n\tBody " << i << ": "
         << robot->GetFreeBody(i)->GetWorldTransformation();
  cout << endl;
}
