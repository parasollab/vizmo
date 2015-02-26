#include "NodeEditDialog.h"

#include <sstream>

#include "GLWidget.h"
#include "MainWindow.h"
#include "ModelSelectionWidget.h"

#include "Models/BoundingBoxModel.h"
#include "Models/BoundingSphereModel.h"
#include "Models/CfgModel.h"
#include "Models/MultiBodyModel.h"
#include "Models/QueryModel.h"
#include "Models/RobotModel.h"
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
  setStyleSheet("QLabel { font:8pt } QLineEdit { font:8pt }");

  QGridLayout* layout = new QGridLayout();
  this->setLayout(layout);

  QLabel* dofName = new QLabel(this);
  dofName->setText(QString::fromStdString(_label));
  layout->addWidget(dofName, 1, 1, 1, 14);

  m_slider = new QSlider(this);
  m_slider->setOrientation(Qt::Horizontal);
  m_slider->installEventFilter(this);
  layout->addWidget(m_slider, 2, 1, 1, 14);

  m_dofValue = new QLineEdit(this);
  layout->addWidget(m_dofValue, 3, 14);

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
    m_tempObjs.AddCfg(m_tempNode);

    //Set temporary edges
    if(GetVizmo().GetMap()) {
      Graph* graph = GetVizmo().GetMap()->GetGraph();
      VI vit = graph->find_vertex(m_originalNode->GetIndex());
      for(EI eit = vit->begin(); eit != vit->end(); ++eit) {
        m_nodesToConnect.push_back((*eit).target());
        EdgeModel* tempEdge = new EdgeModel((*eit).property());
        CfgModel* targetCfg = &(graph->find_vertex((*eit).target())->property());
        tempEdge->Set(m_tempNode, targetCfg);
        m_tempObjs.AddEdge(tempEdge);
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
        m_tempObjs.AddCfg(m_tempNode);
        m_tempObjs.AddEdge(startEdge);
        m_tempObjs.AddEdge(endEdge);
      }
    }
  }

  //Set up dialog widget
  Init();

  //Configure end behavior
  connect(this, SIGNAL(finished(int)), this, SLOT(FinalizeNodeEdit(int)));
}


//Add new node constructor
NodeEditDialog::
NodeEditDialog(MainWindow* _mainWindow, string _title)
    : QDialog(_mainWindow), m_title(_title),
    m_tempNode(NULL), m_originalNode(NULL),
    m_nodesToConnect(), m_nodesToDelete(), m_tempObjs() {
  //Set temporary cfg for editing
  m_tempNode = new CfgModel();
  m_tempObjs.AddCfg(m_tempNode);

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
  m_tempObjs.AddCfg(m_tempNode);

  //Set temporary edges
  Graph* graph = GetVizmo().GetMap()->GetGraph();
  for(vector<VID>::iterator vit = m_nodesToConnect.begin();
      vit != m_nodesToConnect.end(); ++vit) {
    EdgeModel* tempEdge = new EdgeModel();
    CfgModel* targetCfg = &(graph->find_vertex(*vit)->property());
    tempEdge->Set(m_tempNode, targetCfg);
    m_tempObjs.AddEdge(tempEdge);
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
  setFixedWidth(200);
  setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Maximum);

  QLabel* nodeLabel = new QLabel(this);
  QScrollArea* scrollArea = new QScrollArea(this);
  scrollArea->setWidgetResizable(true);

  QVBoxLayout* scrollAreaBoxLayout = new QVBoxLayout();
  scrollAreaBoxLayout->setSpacing(4);
  scrollAreaBoxLayout->setContentsMargins(3, 7, 3, 7); //L, T, R, B

  QGroupBox* scrollAreaBox = new QGroupBox(this);
  scrollAreaBox->setLayout(scrollAreaBoxLayout);

  QDialogButtonBox* okayCancel = new QDialogButtonBox(this);
  okayCancel->setOrientation(Qt::Horizontal);
  okayCancel->setStandardButtons(QDialogButtonBox::Cancel | QDialogButtonBox::Ok);
  connect(okayCancel, SIGNAL(accepted()), this, SLOT(accept()));
  connect(okayCancel, SIGNAL(rejected()), this, SLOT(reject()));

  QVBoxLayout* overallLayout = new QVBoxLayout();
  overallLayout->addWidget(nodeLabel);
  overallLayout->addWidget(scrollArea);
  overallLayout->addWidget(okayCancel);

  SetUpSliders(m_sliders);
  for(SIT it = m_sliders.begin(); it != m_sliders.end(); it++)
    scrollAreaBoxLayout->addWidget(*it);

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
  vector<MultiBodyModel::DOFInfo>& dofInfo = MultiBodyModel::GetDOFInfo();
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
  //Also assumes index alignment
  (*m_tempNode)[_id] = m_sliders[_id]->GetSlider()->value() / 100000.0;

  ValidityCheck();

  if(m_tempNode->IsQuery()) {
    GetVizmo().GetQry()->Build();
    GetVizmo().PlaceRobot();
  }
  GetMainWindow()->GetGLWidget()->updateGL();
}


void
NodeEditDialog::
ValidityCheck() {
  vector<MultiBodyModel::DOFInfo>& dofInfo = MultiBodyModel::GetDOFInfo();
  bool collFound = false; //new or existing
  for(size_t i = 0; i < dofInfo.size(); i++) {
    if(((*m_tempNode)[i] <= dofInfo[i].m_minVal) ||
        ((*m_tempNode)[i] >= dofInfo[i].m_maxVal)) {
      m_tempNode->SetValidity(false);
      collFound = true;
    }
  }
  if(collFound == false) {
    m_tempNode->SetValidity(true);
    GetVizmo().CollisionCheck(*m_tempNode);
  }

  if(m_tempObjs.GetEdges().size() > 0) {
    typedef vector<EdgeModel*>::iterator EIT;
    typedef vector<CfgModel>::iterator IIT;

    for(EIT eit = m_tempObjs.GetEdges().begin();
        eit != m_tempObjs.GetEdges().end(); eit++) {
      //If no intermediates, just check start and end
      double weight = 0.0;
      vector<CfgModel>& intermediates = (*eit)->GetIntermediates();

      if(intermediates.empty()) {
        pair<bool, double> visibility = GetVizmo().VisibilityCheck(
            *(*eit)->GetStartCfg(), *(*eit)->GetEndCfg());
        (*eit)->SetValidity(visibility.first);
        weight = visibility.second;
      }

      else {
        //Check boundary intermediates
        pair<bool, double> startVis = GetVizmo().VisibilityCheck(
            *(*eit)->GetStartCfg(), intermediates[0]);
        (*eit)->SetValidity(startVis.first);
        if(!startVis.first)
          break;
        weight += startVis.second;

        pair<bool, double> endVis = GetVizmo().VisibilityCheck(
            intermediates[intermediates.size() - 1], *(*eit)->GetEndCfg());
        (*eit)->SetValidity(endVis.first);
        if(!endVis.first)
          break;
        weight += endVis.second;

        //Check intermediates in between
        for(IIT it = intermediates.begin(), it2 = it + 1;
            it2 != intermediates.end(); it++, it2++) {
          pair<bool, double> betweenVis = GetVizmo().VisibilityCheck(*it, *it2);
          (*eit)->SetValidity(betweenVis.first);
          if(!betweenVis.first)
            break;
          weight += betweenVis.second;
        }
      }

      (*eit)->SetWeight(weight);
    } //end for all current edges
  }
}


void
NodeEditDialog::
FinalizeNodeEdit(int _accepted) {

  Map* map = GetVizmo().GetMap();

  if(_accepted == 1) {  //user pressed okay
    if(m_tempNode->IsValid()) {
      //set data for original node to match temp
      m_originalNode->SetCfg(m_tempNode->GetDataCfg());

      //delete edges that are no longer valid
      if(map) {
        Graph* graph = map->GetGraph();
        for(vector<EdgeModel*>::iterator eit = m_tempObjs.GetEdges().begin();
            eit != m_tempObjs.GetEdges().end(); ++eit) {
          if((*eit)->IsValid() == false) {
            VID start = map->Cfg2VID(*((*eit)->GetStartCfg()));
            VID end = map->Cfg2VID(*((*eit)->GetEndCfg()));
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
      GetVizmo().PlaceRobot();
    }
  }
  GetMainWindow()->GetModelSelectionWidget()->ResetLists();
  GetMainWindow()->GetGLWidget()->updateGL();
}


void
NodeEditDialog::
FinalizeNodeAdd(int _accepted) {
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
    GetMainWindow()->GetModelSelectionWidget()->ResetLists();
    GetMainWindow()->GetGLWidget()->updateGL();
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
      for(vector<EdgeModel*>::iterator vit = m_tempObjs.GetEdges().begin();
          vit != m_tempObjs.GetEdges().end(); vit++) {
        if((*vit)->IsValid()) {
          graph->add_edge(superID, map->Cfg2VID(*((*vit)->GetEndCfg())));
          graph->add_edge(map->Cfg2VID(*((*vit)->GetEndCfg())), superID);
        }
      }
      //Remove selected vertices
      for(vector<VID>::iterator it = m_nodesToDelete.begin();
          it != m_nodesToDelete.end(); it++)
        graph->delete_vertex(*it);

      map->RefreshMap();
    }
    else
      QMessageBox::about(this, "", "Invalid merge!");
  }
  GetMainWindow()->GetModelSelectionWidget()->ResetLists();
  GetMainWindow()->GetGLWidget()->updateGL();
}
