
#ifndef _STAPL_PERF_DB_H
#  define _STAPL_PERF_DB_H

/**
 * stapl_perf_db.h - this is the "API" for accessing the stapl_perf database
 *
 * 20feb2005/jkp2866 - initial version
**/

#  include <assert.h>
#  include "Defines.h"
#  include "runtime.h"
#  include "db/stapl_perf_db_v1.h"

namespace stapl
{
  class stapl_perf_db:stapl_perf_db_v1
  {

  private:

    timer start;
    int dumped;                 // have we dumped the data yet (currently only one dump per stapl_perf_db)

    typedef struct
    {
      string name;
      string value;
      string interpretation;
    } attribute;
     vector < attribute > attributes;

    void stapl_perf_db_default(string applications_name)
    {
      char hostname[HOST_NAME_MAX];
      struct tm tmtime;
      char stime[15];
      time_t ttime = time(NULL);
      localtime_r(&ttime, &tmtime);
      strftime(stime, 15, "%Y%m%d%k%M%S", &tmtime); // YEAR+MO+DY+HR+MI+SE
      string timestamp(stime);

      dumped = 0;
      assert(gethostname(hostname, HOST_NAME_MAX) == 0);

      /* set some defaults for the run (developers can override as needed) */
      set_attribute("DB:applications_name", applications_name);
      set_attribute("DB:runs_timestamp", timestamp);   // OK if saved internally as string
      set_attribute("DB:machines_name", hostname);
      set_attribute("DB:users_csuser", getlogin());
      set_attribute("PID", (long) getpid());

      // whatever else we can guess
      set_attribute("num_threads", (long) get_num_threads());

      start = start_timer();   // start a default timer (determined by compile-time #defines and run-time environment variables)
    }


    attribute *find_attribute(string attribute_name)
    {
      vector < attribute >::iterator ap;
      for (ap = attributes.begin(); ap != attributes.end(); ap++)
      {
        if (ap->name == attribute_name) {
          return &(*ap);
        }
      }
      return NULL;
    }

  public:

    stapl_perf_db(void) {
      cout <<
          "HELLLLOOOOO... default constructor here... please use me Mr. Compiler"
          << endl;
      string applications_name("unknown");
      stapl_perf_db_default(applications_name);
    }

    stapl_perf_db(string applications_name = "unknown") {
      stapl_perf_db_default(applications_name);
    }

    ~stapl_perf_db(void)
    {
      /* Note: we could stop a default timer and record it before dumping */
      double profile_timer_value = stop_timer(start);   // stop default timer
      set_attribute("db_basic_timer", profile_timer_value);     // and record it and how it was collected
      set_attribute("db_basic_timer_type",
                    "the string from the current, basic, start/stop_timer that tells what it was"
                    /* FIX */ );
      dump();
    }

    void set_attribute(string name, string value, string interpretation)
    {
      attribute *ap = find_attribute(name);
      //cerr << "r_a(" << name << " " << value << " "<< interpretation << ")" << endl;
      if (ap) {
        ap->value = value;      // update with new value
      } else {
        attribute *a = new attribute();
        a->name = name;
        a->value = value;
        a->interpretation = interpretation;
        attributes.push_back(*a);
      }
    }

    void set_attribute(string name, string value)
    {
      string interpretation("STR");
      set_attribute(name, value, interpretation);
    }

    void set_attribute(string name, long value)
    {
      string interpretation("INT");
      char buff[32];
      sprintf(buff, "%ld", value);
      string svalue(buff);
      set_attribute(name, svalue, interpretation);
    }

    void set_attribute(string name, double value)
    {
      string interpretation("REAL");
      char buff[32];
      sprintf(buff, "%lf", value);
      string svalue(buff);
      set_attribute(name, svalue, interpretation);
    }

    void dump(ofstream & o)
    {
      //stapl_perf_db db;
      vector < attribute >::iterator ai;
      attribute *ap, *ts, *a_n, *m_n, *u_c;

      if (dumped)
        return;                 // already dumped

      o << "# SQL for output of " << "[a timestamp]" << "[a pid]" <<
          "[anything else worth putting in comment]" << endl;

      // add statement to add application (insert_application will do nothing if application->name already in DB [with proper indexes])
      a_n = find_attribute("DB:applications_name");
      ap = find_attribute("DB:applications_desc");
      if (a_n) {
        if (ap)
          insert_application(o, a_n->value, ap->value);
        else
          insert_application(o, a_n->value, a_n->value);
      } else {
        o << "# WARNING: unable to add application" << endl;
      }

      // add statement to add machine (insert_machine will do nothing if machines->name already in DB [with proper indexes])
      m_n = find_attribute("DB:machines_name");
      ap = find_attribute("DB:machines_desc");
      if (m_n) {
        if (ap)
          insert_machine(o, m_n->value, ap->value);
        else
          insert_machine(o, m_n->value, m_n->value);
      } else {
        o << "# WARNING: unable to add machine" << endl;
      }

      // add statement to add user (insert_user will do nothing if user->cslogin already in DB [with proper indexes])
      u_c = find_attribute("DB:users_csuser");
      ap = find_attribute("DB:users_name");
      if (u_c) {
        if (ap)
          insert_user(o, u_c->value, ap->value);
        else
          insert_user(o, u_c->value, u_c->value);
      } else {
        o << "# WARNING: unable to add user" << endl;
      }

      // "" add run
      ap = find_attribute("DB:runs_desc");
      ts = find_attribute("DB:runs_timestamp");
      if (ap)
        insert_run(o, m_n->value, a_n->value, u_c->value, ap->value,
                   ts->value);
      else
        insert_run(o, m_n->value, a_n->value, u_c->value,
                   a_n->value /* +m_n->value+u_c->value+ts->value */ ,
                   ts->value);

      // "" add stats
      a_n = find_attribute("DB:applications_name");
      if (!a_n)
        o << "# WARNING: unable to associate statistics with application"
            << endl;
      for (ai = attributes.begin(); ai != attributes.end(); ai++) {
        if (ai->name.substr(0, 3) != "DB:") {   // does not begin with "DB:" 
          // "" add statistics_info
          insert_statistic_info(o, ai->name, 0, ai->name,
                                ai->interpretation);
          // "" add statistics_associations
          if (a_n)
            insert_statistic_association(o, a_n->value, ai->name);
          // "" add statistic
          insert_statistic(o, ai->name, ai->value);
        }
      }
      dumped = 1;
    }

    void dump(string fname)
    {
      if (dumped)
        return;
      ofstream o(fname.c_str());
      dump(o);
      o.close();
    }

    void dump(void)
    {
      attribute *ap;
      string fname = "";

      if (dumped)
        return;                 // already dumped

      ap = find_attribute("DB:applications_name");
      if (ap)
        fname += ap->value;
      ap = find_attribute("DB:machines_name");
      if (ap)
        fname += "-" + ap->value;
      ap = find_attribute("DB:users_csuser");
      if (ap)
        fname += "-" + ap->value;
      ap = find_attribute("DB:runs_timestamp");
      if (ap)
        fname += "-" + ap->value;
      ap = find_attribute("PID");
      if (ap)
        fname += "-" + ap->value;
      fname += ".sql";

      dump(fname);              // default name to dump to
    }
  };
}

#endif
