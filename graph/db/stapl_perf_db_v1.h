
#ifndef _STAPL_PERF_DB_V1_H
#  define _STAPL_PERF_DB_V1_H

/**
 * stapl_perf_db_v1.h - generates SQL code for STAPL MySQL Database "stapl_perf"
 *                      This file should also be used to document the structure
 *                      of the database (including assumptions about indexes)
 *
 * 20feb2005/jkp2866 - initial version
 *
**/

#  include "Defines.h"

#  define ECHO_SQL
#  ifdef ECHO_SQL
#    define TERMINATESQL "\\p;" << endl;
#  else
#    define TERMINATESQL ";" << endl;
#  endif

namespace stapl
{
  class stapl_perf_db_v1
  {

  public:

    static const double API = 1.0;

  protected:
    // assumes "applications" has a unique index on "name"
    void insert_application(ostream & o, string & applications_name,
                            string & applications_description)
    {
      o << "INSERT IGNORE INTO applications "
          << "VALUES (0, '"
          << applications_name << "', '"
          << applications_description << "')" << TERMINATESQL;
    }

    // delete_applications should probably remove all stat/group associations as well
    // delete_applications should probably remove all runs for that application as well (??? that's rather scary ???)
    void delete_application(ostream & o, string & applications_name)
    {
      o << "DELETE FROM applications "
          << "WHERE name='" << applications_name << "' " << TERMINATESQL;
    }

    void delete_application(ostream & o, int applications_app_id)
    {
      o << "DELETE FROM applications "
          << "WHERE app_id='" << applications_app_id << "' "
          << TERMINATESQL;
    }

    // assumes "groups" has a unique index on "name"
    void insert_group(ostream & o, string & groups_name)
    {
      o << "INSERT IGNORE INTO groups "
          << "VALUES (0, '" << groups_name << "') " << TERMINATESQL;
    }

    // delete_group should probably remove all group associations as well
    void delete_group(ostream & o, string & groups_name)
    {
      o << "DELETE FROM groups "
          << "WHERE name='" << groups_name << "' " << TERMINATESQL;
    }

    void delete_group(ostream & o, int groups_group_id)
    {
      o << "DELETE FROM groups "
          << "WHERE group_id='" << groups_group_id << "' " << TERMINATESQL;
    }

    // assumes group_associations has a unique index on "app_id + group_id"
    void insert_group_association(ostream & o, string & applications_name,
                                  string & groups_name)
    {
      o << "INSERT IGNORE INTO groups_associations "
          << "SELECT applications.app_id, groups.group_id "
          << "FROM applications, groups "
          << "WHERE applications.name='" << applications_name << "' "
          << "AND groups.name='" << groups_name << "' " << TERMINATESQL;
    }

    // currently independent (no further deletes needed)
    void delete_group_association(ostream & o, string & applications_name,
                                  string & groups_name)
    {
#  ifdef HAVE_SQL4PLUS
      o << "DELETE FROM groups_associations "
          << "USING applications, groups "
          << "WHERE applications.name='" << applications_name << "' "
          << "AND groups.name='" << groups_name << "' "
          << "AND groups_associations.app_id=applications.app_id "
          << "AND groups_associations.group_id=groups.group_id "
          << TERMINATESQL;
#  else
      o << "CREATE TEMPORARY TABLE tmp "
          << "SELECT groups_associations.* "
          << "FROM groups_associations, applications, groups "
          << "WHERE applications.name='" << applications_name << "' "
          << "AND groups.name='" << groups_name << "' "
          << "AND groups_associations.app_id=applications.app_id "
          << "AND groups_associations.group_id=groups.group_id "
          << TERMINATESQL;
      o << "ALTER TABLE groups_associations "
          << "ADD deleteme INT(1) NOT NULL DEFAULT '0' " << TERMINATESQL;
      o << "REPLACE groups_associations "
          << "SELECT *,'1' FROM tmp " << TERMINATESQL;
      o << "DELETE FROM groups_associations "
          << "WHERE deleteme='1' " << TERMINATESQL;
      o << "ALTER TABLE groups_associations "
          << "DROP deleteme " << TERMINATESQL;
      o << "DROP TABLE tmp " << TERMINATESQL;
#  endif
    }

    // assumes "machines" has a unique index on "name"
    void insert_machine(ostream & o, string & machines_name,
                        string & machines_description)
    {
      o << "INSERT IGNORE INTO machines "
          << "VALUES (0, '"
          << machines_name << "', '"
          << machines_description << "') " << TERMINATESQL;
    }

    // delete_machine should probably remove all runs for that machine as well (??? that's rather scary ???)
    void delete_machine(ostream & o, string & machines_name)
    {
      o << "DELETE FROM machines "
          << "WHERE machines.name='" << machines_name << "' "
          << TERMINATESQL;
    }

    void delete_machine(ostream & o, int &machines_machine_id)
    {
      o << "DELETE FROM machines "
          << "WHERE machines.machine_id='" << machines_machine_id << "' "
          << TERMINATESQL;
    }

    // assumes "statistics_info" has a unique index on "name"
    void insert_statistic_info(ostream & o, string & statistics_info_name,
                               int statistics_info_fixed,
                               string & statistics_info_description,
                               string & statistics_info_interpretation)
    {
      o << "INSERT IGNORE INTO statistics_info "
          << "VALUES (0, '"
          << statistics_info_name << "', '"
          << statistics_info_fixed << "', '"
          << statistics_info_description << "', '"
          << statistics_info_interpretation << "') " << TERMINATESQL;
    }

    void delete_statistic_info(ostream & o, string & statistics_info_name)
    {
      o << "DELETE FROM statistics_info "
          << "WHERE statistics_info.name='" << statistics_info_name << "' "
          << TERMINATESQL;
    }

    void delete_statistic_info(ostream & o, int &statistics_info_stat_id)
    {
      o << "DELETE FROM statistics_info "
          << "WHERE statistics_info.stat_id='" << statistics_info_stat_id
          << "' " << TERMINATESQL;
    }

    // assumes "statistics_associations" has a unique index on "app_id + stat_id"
    void insert_statistic_association(ostream & o,
                                      string & applications_name,
                                      string & statistics_info_name)
    {
      o << "INSERT IGNORE INTO statistics_associations "
          << "SELECT applications.app_id, statistics_info.stat_id "
          << "FROM applications, statistics_info "
          << "WHERE applications.name='" << applications_name << "' "
          << "AND statistics_info.name='" << statistics_info_name << "' "
          << TERMINATESQL;
    }

    // currently independent (no further deletes needed)
    void delete_statistic_association(ostream & o,
                                      string & applications_name,
                                      string & statistics_info_name)
    {
#  ifdef HAVE_SQL4PLUS
      o << "DELETE FROM statistics_associations USING applications, statistics_infos WHERE applications.name='" << applications_name << "' AND statistics_info.name='" << statistics_info_name << "' AND statistics_associations.app_id=applications.app_id AND statistics_associations.stat_id=statistic_info.stat_id;" << endl;
#  else
      o << "CREATE TEMPORARY TABLE tmp "
          << "SELECT statistics_associations.* "
          << "FROM statistics_associations, applications, statistics_info "
          << "WHERE applications.name='" << applications_name << "' "
          << "AND statistics_info.name='" << statistics_info_name << "' "
          << "AND statistics_associations.app_id=applications.app_id "
          << "AND statistics_associations.stat_id=statistics_info.stat_id "
          << TERMINATESQL;
      o << "ALTER TABLE statistics_associations "
          << "ADD deleteme INT(1) NOT NULL DEFAULT '0' " << TERMINATESQL;
      o << "REPLACE statistics_associations "
          << "SELECT *,'1' FROM tmp " << TERMINATESQL;
      o << "DELETE FROM statistics_associations "
          << "WHERE deleteme='1' " << TERMINATESQL;
      o << "ALTER TABLE statistics_associations "
          << "DROP deleteme " << TERMINATESQL;
      o << "DROP TABLE tmp " << TERMINATESQL;
#  endif
    }

    // assumes "users" has a unique index on "csuser"
    void insert_user(ostream & o, string & users_csuser,
                     string & users_name)
    {
      o << "INSERT IGNORE INTO users "
          << "VALUES (0, '"
          << users_csuser << "', '" << users_name << "') " << TERMINATESQL;
    }

    void delete_user(ostream & o, string & users_csuser)
    {
      o << "DELETE FROM users "
          << "WHERE users.csuser='" << users_csuser << "' "
          << TERMINATESQL;
    }

    void delete_user(ostream & o, string & users_csuser,
                     string & users_name)
    {
      o << "DELETE FROM users "
          << "WHERE users.csuser='" << users_csuser << "'"
          << "AND users.name'" << users_name << "' " << TERMINATESQL;
    }

    void delete_user(ostream & o, int &users_user_id)
    {
      o << "DELETE FROM users "
          << "WHERE users.user_id='" << users_user_id << "' "
          << TERMINATESQL;
    }

    // assumes "runs" has a unique index on "machine_id + application_id + user_id + timestamp" (???? valid assumption ????)
    void insert_run(ostream & o, string & machines_name,
                    string & applications_name, string & users_csuser,
                    string & runs_description, string & runs_timestamp)
    {
      o << "INSERT IGNORE INTO runs "
          <<
          "SELECT 0, machines.machine_id, applications.app_id, users.user_id,  '"
          << runs_description << "', '" << runs_timestamp << "' " <<
          "FROM machines, applications, users " << "WHERE machines.name='"
          << machines_name << "' " << "AND applications.name='" <<
          applications_name << "' " << "AND users.csuser='" << users_csuser
          << "' " << TERMINATESQL;
    }

    void delete_run(ostream & o, string & machines_name,
                    string & applications_name, string & users_csuser,
                    string & runs_description, string & runs_timestamp)
    {
      o << "" << endl;
    }

    void delete_run(ostream & o, int run_id)
    {
      o << endl;
    }

    // assumes "statistics" has a unique index on "run_id + stat_id" (???? valid assumption ????)
    void insert_statistic(ostream & o, string & statistics_info_name,
                          string & statistics_value)
    {
      // *** IMPORTANT!!! - this assumes that this is being called immediately after the insert_run [because of last_insert_id()] ***
      o << "INSERT IGNORE INTO statistics "
          << "SELECT last_insert_id(), statistics_info.stat_id, '" <<
          statistics_value << "' " << "FROM statistics_info " <<
          "WHERE statistics_info.name='" << statistics_info_name << "' " <<
          TERMINATESQL;
    }

    void delete_statistic(ostream & o, int runs_run_id,
                          string & statistics_info_name)
    {
#  ifdef HAVE_SQL4PLUS
      o << "DELETE FROM statistics "
          << "USING runs, statistics_info "
          << "WHERE runs.run_id='" << runs_run_id << "' "
          << "AND statistics_info.name='" << statistics_info_name << "' "
          << "AND statistics.stat_id=statistics_info.stat_id "
          << TERMINATESQL;
#  else
      o << "# write me" << TERMINATESQL;
#  endif
    }

    void delete_statistic(ostream & o, string & statistics_info_name,
                          string & runs_timestamp)
    {
#  ifdef HAVE_SQL4PLUS
      o << "DELETE FROM statistics "
          << "USING runs, statistics_info "
          << "WHERE runs.timestamp='" << runs_timestamp << "' "
          << "AND statistics_info.name='" << statistics_info_name << "' "
          << "AND statistics.stat_id=statistics_info.stat_id "
          << TERMINATESQL;
#  else
      o << "# write me" << TERMINATESQL;
#  endif
    }

    void delete_statistic(ostream & o, int &runs_run_id,
                          int &statistics_info_stat_id)
    {
#  ifdef HAVE_SQL4PLUS
      o << "DELETE FROM statistics "
          << "USING runs, statistics_info "
          << "WHERE runs.run_id='" << runs_run_id << "' "
          << "AND statistics_info.stat_id='" << statistics_info_stat_id <<
          "' " << TERMINATESQL;
#  else
      o << "# write me" << TERMINATESQL;
#  endif
    }

  public:

    void test_insert(string applications_name,
                     string applications_description, string groups_name,
                     string machines_name, string machines_description,
                     string runs_description, string runs_timestamp,
                     string statistics_value, string statistics_info_name,
                     int statistics_info_fixed,
                     string statistics_info_description,
                     string statistics_info_interpretation,
                     string users_csuser, string users_name)
    {

      cout << endl << "# test insert_ methods" << endl;
      insert_application(cout, applications_name,
                         applications_description);
      insert_group(cout, groups_name);
      insert_group_association(cout, applications_name, groups_name);
      insert_machine(cout, machines_name, machines_description);
      insert_statistic_info(cout, statistics_info_name,
                            statistics_info_fixed,
                            statistics_info_description,
                            statistics_info_interpretation);
      insert_statistic_association(cout, applications_name,
                                   statistics_info_name);
      insert_user(cout, users_csuser, users_name);
      insert_run(cout, machines_name, applications_name, users_csuser,
                 runs_description, runs_timestamp);
      insert_statistic(cout, statistics_info_name, statistics_value);
    }

    void test_select(string applications_name,
                     string applications_description, string groups_name,
                     string machines_name, string machines_description,
                     string runs_description, string runs_timestamp,
                     string statistics_value, string statistics_info_name,
                     int statistics_info_fixed,
                     string statistics_info_description,
                     string statistics_info_interpretation,
                     string users_csuser, string users_name)
    {
      cout << "SELECT * FROM applications "
          << "WHERE name='" << applications_name << "' " << TERMINATESQL;
      cout << "SELECT * FROM groups "
          << "WHERE name='" << groups_name << "' " << TERMINATESQL;
      cout << "SELECT * FROM groups_associations, applications, groups "
          << "WHERE applications.name='" << applications_name << "' "
          << "AND groups.name='" << groups_name << "' "
          << "AND groups_associations.app_id=applications.app_id "
          << "AND groups_associations.group_id=groups.group_id "
          << TERMINATESQL;
      cout << "SELECT * FROM machines "
          << "WHERE name='" << machines_name << "' " << TERMINATESQL;
      cout << "SELECT * FROM statistics_info "
          << "WHERE name='" << statistics_info_name << "' "
          << TERMINATESQL;
      cout <<
          "SELECT * FROM statistics_associations, applications, statistics_info "
          << "WHERE applications.name='" << applications_name << "' " <<
          "AND statistics_info.name='" << statistics_info_name << "' " <<
          "AND statistics_associations.app_id=applications.app_id " <<
          "AND statistics_associations.stat_id=statistics_info.stat_id " <<
          TERMINATESQL;
      cout << "SELECT * FROM users " << "WHERE csuser='" << users_csuser <<
          "'" << TERMINATESQL;
      cout << "SELECT * FROM runs, machines, applications, users " <<
          "WHERE runs.timestamp='" << runs_timestamp << "' " <<
          "AND machines.name='" << machines_name << "' " <<
          "AND applications.name='" << applications_name << "' " <<
          "AND users.name='" << users_name << "' " <<
          "AND runs.machine_id=machines.machine_id " <<
          "AND runs.application_id=applications.app_id " <<
          "AND runs.user_id=users.user_id " << TERMINATESQL;
      cout << "SELECT * FROM statistics, runs, statistics_info " <<
          "WHERE runs.timestamp='" << runs_timestamp << "' " <<
          "AND statistics_info.name='" << statistics_info_name << "' " <<
          "AND statistics.run_id=runs.run_id " <<
          "AND statistics.stat_id=statistics_info.stat_id " <<
          TERMINATESQL;
    }

    void test_delete(string applications_name,
                     string applications_description, string groups_name,
                     string machines_name, string machines_description,
                     string runs_description, string runs_timestamp,
                     string statistics_value, string statistics_info_name,
                     int statistics_info_fixed,
                     string statistics_info_description,
                     string statistics_info_interpretation,
                     string users_csuser, string users_name)
    {
      cout << endl << "# test delete_ methods" << endl;
      //delete_statistic(cout, statistics_info_name, statistics_value);
      //delete_run(cout, machines_name, applications_name, users_csuser, runs_description, runs_timestamp);
      delete_user(cout, users_csuser);
      delete_statistic_association(cout, applications_name,
                                   statistics_info_name);
      delete_statistic_info(cout, statistics_info_name);
      delete_machine(cout, machines_name);
      delete_group_association(cout, applications_name, groups_name);
      delete_group(cout, groups_name);
      delete_application(cout, applications_name);
    }

  };                            // end class stapl_perf_db
}                               // end namespace

#endif

// EOF - stapl_perf_db_v1.h
