#ifndef CONTROL_MODEL_H_
#define CONTROL_MODEL_H_

#include "Utilities/IO.h"

class MultiBodyModel;

class ControlModel {
  public:

    ControlModel(MultiBodyModel* _owner);

    const vector<double>& GetControl() const {return m_control;}

    void Read(istream& _is);

    friend ostream& operator<<(ostream& _os, const ControlModel& _c);

  private:
    MultiBodyModel* m_owner;
    vector<double> m_control;
};

#endif
