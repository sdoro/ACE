// -*- C++ -*-
// $Id: QCTQM_Test_Receiver_exec.cpp 95840 2012-06-07 19:12:50Z johnnyw $

/**
 * Code generated by the The ACE ORB (TAO) IDL Compiler v1.8.3
 * TAO and the TAO IDL Compiler have been developed by:
 *       Center for Distributed Object Computing
 *       Washington University
 *       St. Louis, MO
 *       USA
 *       http://www.cs.wustl.edu/~schmidt/doc-center.html
 * and
 *       Distributed Object Computing Laboratory
 *       University of California at Irvine
 *       Irvine, CA
 *       USA
 * and
 *       Institute for Software Integrated Systems
 *       Vanderbilt University
 *       Nashville, TN
 *       USA
 *       http://www.isis.vanderbilt.edu/
 *
 * Information about TAO is available at:
 *     http://www.cs.wustl.edu/~schmidt/TAO.html
 **/

#include "QCTQM_Test_Receiver_exec.h"
#include "tao/ORB_Core.h"
#include "ace/Reactor.h"
#include "ace/OS_NS_unistd.h"

#include "dds4ccm/impl/dds4ccm_conf.h"
#include "dds4ccm/impl/Utils.h"
#include "dds4ccm/impl/dds4ccm_utils.h"

#define QUERY "( (iteration > %0) AND (iteration < %1) )"

//run 1
#define MIN_ITERATION_1 "2"
#define MAX_ITERATION_1 "5"
//run 2
#define MIN_ITERATION_2 "22"
#define MAX_ITERATION_2 "34"
//run 4
#define MIN_ITERATION_3 "68"
#define MAX_ITERATION_3 "77"

// Reader also reads already read samples.
// The getter receives the following iterations:
// During run 1: 2 (iterations 3 and 4)
// During run 2: 11 (iterations between 22 and 34)
// During run 3: 47 (all unread samples, meaning iterations 1-60
//               without iteration 3, 4 and iterations between 22 and 34)
// During run 4: 8 (iterations between 68 and 77)

#define SAMPLES_PER_KEY_GETTER (2 + 11 + 47 + 8)

namespace CIAO_QCTQM_Test_Receiver_Impl
{
  /**
   * Read action generator
   */

  read_action_Generator::read_action_Generator (Receiver_exec_i &callback, int run)
    : callback_ (callback),
      run_ (run)
  {
  }

  read_action_Generator::~read_action_Generator ()
  {
  }

  int
  read_action_Generator::handle_timeout (const ACE_Time_Value &, const void *)
  {
    ACE_DEBUG ((LM_DEBUG, "Checking if last sample "
                          "is available in DDS...\n"));
    if (this->callback_.check_last ())
      {
        this->callback_.run (this->run_);
      }
    return 0;
  }

  /**
   * Facet Executor Implementation Class: get_port_status_exec_i
   */

  get_port_status_exec_i::get_port_status_exec_i (
        ::QCTQM_Test::CCM_Receiver_Context_ptr ctx)
    : ciao_context_ (
        ::QCTQM_Test::CCM_Receiver_Context::_duplicate (ctx))
  {
  }

  get_port_status_exec_i::~get_port_status_exec_i (void)
  {
  }

  // Operations from ::CCM_DDS::PortStatusListener

  void
  get_port_status_exec_i::on_requested_deadline_missed (::DDS::DataReader_ptr /* the_reader */,
  const ::DDS::RequestedDeadlineMissedStatus & /* status */)
  {
    /* Your code here. */
  }

  void
  get_port_status_exec_i::on_sample_lost (::DDS::DataReader_ptr /* the_reader */,
  const ::DDS::SampleLostStatus & /* status */)
  {
    /* Your code here. */
  }

  /**
   * Facet Executor Implementation Class: reader_start_exec_i
   */

  reader_start_exec_i::reader_start_exec_i (
        ::QCTQM_Test::CCM_Receiver_Context_ptr ctx,
        Receiver_exec_i & callback)
    : ciao_context_ (
        ::QCTQM_Test::CCM_Receiver_Context::_duplicate (ctx))
      , callback_ (callback)
  {
  }

  reader_start_exec_i::~reader_start_exec_i (void)
  {
  }

  // Operations from ::TwoQueriesStarter

  void
  reader_start_exec_i::set_reader_properties (::CORBA::UShort nr_keys,
  ::CORBA::UShort nr_iterations)
  {
    ACE_DEBUG ((LM_DEBUG, "Set reader propeties nr_keys %u, nr_iterations %u\n",
                nr_keys, nr_iterations));

    this->callback_.keys (nr_keys);
    this->callback_.iterations (nr_iterations);
  }

  void
  reader_start_exec_i::start_read (::CORBA::UShort run)
  {
    this->callback_.start_read (run);
  }

  /**
   * Component Executor Implementation Class: Receiver_exec_i
   */

  Receiver_exec_i::Receiver_exec_i (void)
  : iterations_ (20)
      , keys_ (5)
      , current_min_iteration_ (ACE_OS::atoi (MIN_ITERATION_1))
      , current_max_iteration_ (ACE_OS::atoi (MAX_ITERATION_1))
      , ticker_ (0)
      , samples_expected_ (0)
      , samples_received_ (0)
  {
  }

  Receiver_exec_i::~Receiver_exec_i (void)
  {
  }

  // Supported operations and attributes.
  ACE_Reactor*
  Receiver_exec_i::reactor (void)
  {
    ACE_Reactor* reactor = 0;
    ::CORBA::Object_var ccm_object =
      this->ciao_context_->get_CCM_object();
    if (! ::CORBA::is_nil (ccm_object.in ()))
      {
        ::CORBA::ORB_var orb = ccm_object->_get_orb ();
        if (! ::CORBA::is_nil (orb.in ()))
          {
            reactor = orb->orb_core ()->reactor ();
          }
      }
    if (reactor == 0)
      {
        throw ::CORBA::INTERNAL ();
      }
    return reactor;
  }

  // check if last key is received, if so we can assume
  // that other key's are arrived too in Receiver.
  bool
  Receiver_exec_i::check_last ()
  {
     ::QCTQM_Test::QueryConditionTestConnector::Reader_var reader =
      this->ciao_context_->get_connection_get_port_data ();

    try
      {
        QueryConditionTest queryfiltertest_info;
        ::CCM_DDS::ReadInfo readinfo;
        char key[10];
        ACE_OS::sprintf (key, "KEY_%d", this->keys_);

        queryfiltertest_info.symbol = ::CORBA::string_dup (key);
        reader->read_one_last (
                queryfiltertest_info,
                readinfo,
                ::DDS::HANDLE_NIL);
        ACE_DEBUG ((LM_DEBUG, "Receiver_exec_i::check_last - "
                              "last iteration <%02d> - <%02d> this->keys_ %s\n",
                               queryfiltertest_info.iteration,
                               this->current_max_iteration_ - 1, key));
        return queryfiltertest_info.iteration >= this->current_max_iteration_ - 1;
      }
    catch (const ::CCM_DDS::InternalError &)
      {
      }
    catch (const ::CCM_DDS::NonExistent &)
      {
      }
    catch (...)
      {
        ACE_ERROR ((LM_ERROR, "Receiver_exec_i::check_last: "
                              "ERROR: Unexpected exception caught\n"));
      }
    return false;
  }

  // Supported operations and attributes.

  // Check for correct iteration belonging to a run.
  // If info == 0, check iteration after a get,
  // else check iteration after a read
  void
  Receiver_exec_i::check_iter (const QueryConditionTest & sample,
                               ::CORBA::UShort run,
                               ::CCM_DDS::ReadInfo * info)
  {
    if (run == 3)
      {
        // We need to receive all UNread samples. Therefore we should
        // receive all samples except the ones between
        // MIN_ITERATION_1 and MAX_ITERATION_1 and between
        // MIN_ITERATION_2 and MAX_ITERATION_2
        if ((sample.iteration > ACE_OS::atoi (MIN_ITERATION_1)  &&
             sample.iteration < ACE_OS::atoi (MAX_ITERATION_1)) ||
            (sample.iteration > ACE_OS::atoi (MIN_ITERATION_2)  &&
             sample.iteration < ACE_OS::atoi (MAX_ITERATION_2)))
          {
            // Read supplies info. There check the sample status mask
            // as well
            if (info != 0)
              { // access mask should be "ALREADY_SEEN" since the getter should
                // already have seen this sample.
                if (info->access_status != ::CCM_DDS::ALREADY_SEEN)
                  {
                    // READ ALL since this check is only performed
                    // during a read.
                    ACE_ERROR ((LM_ERROR, "ERROR: READ ALL: "
                                "Unexpected sample access mask - "
                                "expected <%C> - "
                                "received <%C>\n",
                                CIAO::DDS4CCM::translate_ccm_dds_accessstatus (::CCM_DDS::ALREADY_SEEN),
                                CIAO::DDS4CCM::translate_ccm_dds_accessstatus (info->access_status)
                                ));
                  }
              }
            else
              {
                // Getter functionality
                ACE_ERROR ((LM_ERROR, "ERROR: GET ALL: "
                                      "Didn't except samples between "
                                      "<%02d> and <%02d> and between "
                                      "<%02d> and <%02d>\n",
                                      ACE_OS::atoi (MIN_ITERATION_1),
                                      ACE_OS::atoi (MAX_ITERATION_1),
                                      ACE_OS::atoi (MIN_ITERATION_2),
                                      ACE_OS::atoi (MAX_ITERATION_2)));
              }
          }
      }
    else  //run 1,2, and 4
      {
        //after get
        if (!info)
          {
            if (sample.iteration <= current_min_iteration_)
              {
                ACE_ERROR ((LM_ERROR, "ERROR: GET ALL: "
                                      "Didn't expect samples with iterations "
                                      "<= %02d\n",
                                      this->current_min_iteration_));
              }
            if (sample.iteration > this->current_max_iteration_)
              {
                ACE_ERROR ((LM_ERROR, "ERROR: GET ALL: "
                                      "Didn't expect samples with iterations "
                                      "> %02d\n",
                                      this->current_max_iteration_));
              }
          }
        // after read
        else if (info != 0)
          { // access mask should be "FRESH_INFO" since the getter has not
            // "seen" this sample.
            if (sample.iteration > this->current_min_iteration_)
              {
                if (info->access_status != ::CCM_DDS::FRESH_INFO)
                  {
                    // READ ALL since this check is only performed
                    // during a read.
                    ACE_ERROR ((LM_ERROR, "ERROR: READ ALL: "
                                "Unexpected sample access mask - "
                                "expected <%C> - "
                                "received <%C>\n",
                                CIAO::DDS4CCM::translate_ccm_dds_accessstatus (::CCM_DDS::FRESH_INFO),
                                CIAO::DDS4CCM::translate_ccm_dds_accessstatus (info->access_status)
                                ));
                  }
              }
          }
      }
  }


  void
  Receiver_exec_i::read_all (::CORBA::UShort run)
  {
    ::QCTQM_Test::QueryConditionTestConnector::Reader_var reader =
      this->ciao_context_->get_connection_read_port_data ();

    if (::CORBA::is_nil (reader.in ()))
      {
        ACE_ERROR ((LM_ERROR, "Receiver_exec_i::read_all - "
                              "ERROR: No Reader\n"));
        return;
      }
    QueryConditionTestSeq qf_info;
    ::CCM_DDS::ReadInfoSeq readinfos;
    ACE_DEBUG ((LM_DEBUG, "Receiver_exec_i::read_all - "
                "Start checking samples in DDS\n"));
    reader->read_all (qf_info, readinfos);
    if (qf_info.length () !=
        static_cast < ::CORBA::ULong > (run * this->iterations_ * this->keys_))
      {
        ACE_ERROR ((LM_ERROR, "ERROR: Receiver_exec_i::read_all - "
                              "Unexpected number of samples received: "
                              "expected <%d> - received <%u>\n",
                              run * this->iterations_, qf_info.length ()));
      }
    for (::CORBA::ULong i = 0; i < qf_info.length (); ++i)
      {
 /*     ACE_DEBUG ((LM_DEBUG, "READ ALL : Receiver_exec_i::read_all - "
                              "Sample received: key <%C> - iteration <%d> - "
                              "sample_read_state <%d>\n",
                              qf_info[i].symbol.in (),
                              qf_info[i].iteration,
                              readinfos[i].access_status));
 */
        this->check_iter (qf_info[i], run, &readinfos[i]);
      }
  }


  ::CORBA::ULong
  Receiver_exec_i::get_all (::CORBA::UShort run)
  {

    ACE_DEBUG ((LM_DEBUG, ACE_TEXT ("GET ALL  run %d: \n"), run));
    ACE_OS::sleep (3);

    ::QCTQM_Test::QueryConditionTestConnector::Getter_var getter =
      this->ciao_context_->get_connection_get_port_fresh_data ();
    ::CORBA::ULong samples_received = 0;

    if (::CORBA::is_nil (getter.in ()))
      {
        ACE_ERROR ((LM_ERROR, "Receiver_exec_i::get_all - "
                              "ERROR: No Getter\n"));
        return 0;
      }
    DDS::Duration_t to;
    to.sec = 10;
    to.nanosec = 0;

    getter->time_out (to);
    getter->max_delivered_data (0);
    QueryConditionTestSeq gettertest_seq;
    ::CCM_DDS::ReadInfoSeq readinfo;
    bool const res = getter->get_many (gettertest_seq, readinfo);
    if (res)
      {
        if (gettertest_seq.length () == 0)
          {
            ACE_ERROR ((LM_ERROR, "Receiver_exec_i::get_many: "
                                   "No data returned. "
                                   "number of samples: "
                                   "expected at least one - received <0>, res = %u\n", res));
          }
        for (CORBA::ULong i = 0; i < gettertest_seq.length (); ++i)
          {
            ACE_DEBUG ((LM_DEBUG, "Receiver_exec_i::get_many: "
                                  "Returned data : key <%C> - iteration <%d>,"
                                  "instance status <%u> - access status <%C>\n",
                                  gettertest_seq[i].symbol.in (),
                                  gettertest_seq[i].iteration,
                                  readinfo[i].instance_status,
                                  CIAO::DDS4CCM::translate_ccm_dds_accessstatus(readinfo[i].access_status)));
            this->check_iter (gettertest_seq[i], run);
            ++samples_received;
          }
       }
     else
       {
         ACE_ERROR ((LM_ERROR, "ERROR: GET MANY: "
                               "Time out occurred\n"));
       }
    return samples_received;
  }

  ::CORBA::ULong
  Receiver_exec_i::test_all (::CORBA::UShort run)
  {
    try
      {
        return get_all (run);
      }
    catch (const CCM_DDS::NonExistent& ex)
      {
        for (::CORBA::ULong i = 0; i < ex.indexes.length (); ++i)
          {
            ACE_DEBUG ((LM_DEBUG, ACE_TEXT ("ERROR test_all <%d>: ")
                  ACE_TEXT ("caught expected exception: index <%u>\n"),
                  run,
                  ex.indexes[i]));
          }
      }
    catch (const CCM_DDS::InternalError& ex)
      {
        ACE_ERROR ((LM_ERROR, ACE_TEXT ("ERROR: test_all <%d>: ")
              ACE_TEXT ("caught InternalError exception: retval <%u>\n"),
              run,
              ex.error_code));
      }
    catch (const ::CORBA::Exception& ex)
      {
        ex._tao_print_exception ("test_all");
        ACE_ERROR ((LM_ERROR,
          ACE_TEXT ("ERROR: Receiver_exec_i::test_all <%d> : Exception caught\n"),
          run));
      }
    return 0;
  }

  void
  Receiver_exec_i::check_filter (::CORBA::UShort run)
  {
    ::CCM_DDS::QueryFilter_var filter;
    ::QCTQM_Test::QueryConditionTestConnector::Reader_var reader =
      this->ciao_context_->get_connection_get_port_data ();
    try
      {
        filter = reader->query ();
      }
    catch (const CCM_DDS::InternalError& ex)
      {
        if (run == 3)
          {
            ACE_DEBUG ((LM_DEBUG, "Receiver_exec_i::check_filter - "
                                  "caught expected InternalEr excep: retval <%u>\n",
                                  ex.error_code));
          }
        else
          {
            ACE_ERROR ((LM_ERROR, "ERROR: Receiver_exec_i::check_filter - "
                                  "caught InternalError exception: retval <%u>\n",
                                  ex.error_code));
          }
        return;
      }
    catch (const ::CORBA::Exception& ex)
      {
        ex._tao_print_exception ("ERROR: Receiver_exec_i::check_filter: ");
        ACE_ERROR ((LM_ERROR, "ERROR: Receiver_exec_i::check_filter - "
                              "Exception caught\n"));
        return;
      }

    //check query
    bool error = false;

    if (run != 3)
      {
        if (ACE_OS::strcmp (filter->expression, QUERY) != 0)
          {
            ACE_ERROR ((LM_ERROR, "ERROR: Receiver_exec_i::check_filter - "
                                  "Unexpected query when retrieving filter: "
                                  "expected <%C> - received <%C>\n",
                                  QUERY, filter->expression.in ()));
            error = true;
          }
        //check current parameters.
        if (filter->parameters.length () != 2)
          {
            ACE_ERROR ((LM_ERROR, "ERROR: Receiver_exec_i::check_filter - "
                                  "Unexpected number of parameters: "
                                  "expected <%d> - received <%d>\n",
                                  2, filter->parameters.length ()));
            error = true;
          }

        if (filter->parameters.length () >= 1)
          {
            if (ACE_OS::atoi (filter->parameters[0]) != this->current_min_iteration_)
              {
                ACE_ERROR ((LM_ERROR, "ERROR: Receiver_exec_i::check_filter - "
                                      "Unexpected param value: "
                                      "expected <%d> - received <%C>\n",
                                      this->current_min_iteration_,
                                      filter->parameters[0].in ()));
                error = true;
              }
          }
        if (filter->parameters.length () >= 2)
          {
            if (ACE_OS::atoi (filter->parameters[1]) != this->current_max_iteration_)
              {
                ACE_ERROR ((LM_ERROR, "ERROR: Receiver_exec_i::check_filter - "
                                      "Unexpected param value: "
                                      "expected <%d> - received <%C>\n",
                                      this->current_max_iteration_,
                                      filter->parameters[1].in ()));
                error = true;
              }
          }
      }
    if (!error)
      {
        ACE_DEBUG ((LM_DEBUG, "Receiver_exec_i::check_filter - "
                              "Passed check_filter test for run <%d>.\n",
                              run));
      }
  }

  void
  Receiver_exec_i::set_filter (::CORBA::UShort run)
  {
    ACE_DEBUG ((LM_DEBUG, "Set filter for run <%d>\n", run));
    ::QCTQM_Test::QueryConditionTestConnector::Reader_var reader =
      this->ciao_context_->get_connection_get_port_data ();

    try
      {
        ::CCM_DDS::QueryFilter filter;
        run != 3 ? filter.expression = ::CORBA::string_dup (QUERY) :
                    filter.expression = ::CORBA::string_dup ("");

        filter.parameters.length (2);
        if (run == 1)
          {
            filter.parameters[0] = ::CORBA::string_dup (MIN_ITERATION_1);
            filter.parameters[1] = ::CORBA::string_dup (MAX_ITERATION_1);
            this->current_min_iteration_ = ACE_OS::atoi (MIN_ITERATION_1);
            this->current_max_iteration_ = ACE_OS::atoi (MAX_ITERATION_1);
          }
        else if (run == 2)
          {
            filter.parameters[0] = ::CORBA::string_dup (MIN_ITERATION_2);
            filter.parameters[1] = ::CORBA::string_dup (MAX_ITERATION_2);
            this->current_min_iteration_ = ACE_OS::atoi (MIN_ITERATION_2);
            this->current_max_iteration_ = ACE_OS::atoi (MAX_ITERATION_2);
          }
        else if (run == 3)
          {
            // get all not yet seen by the getter samples of run 1, 2 and 3.
            this->current_min_iteration_ = 1;
            this->current_max_iteration_ = run * this->iterations_;
            filter.parameters[0] = ::CORBA::string_dup (0);
            filter.parameters[1] = ::CORBA::string_dup (0);
          }
        else if (run == 4)
          {
            filter.parameters[0] = ::CORBA::string_dup (MIN_ITERATION_3);
            filter.parameters[1] = ::CORBA::string_dup (MAX_ITERATION_3);
            this->current_min_iteration_ = ACE_OS::atoi (MIN_ITERATION_3);
            this->current_max_iteration_ = ACE_OS::atoi (MAX_ITERATION_3);
          }
        ACE_DEBUG ((LM_DEBUG, "Filter : Query <%C>, parameter[0] <%C>, parameter[1] <%C>\n",
              filter.expression.in (), filter.parameters[0].in (), filter.parameters[1].in ()));
        reader->query (filter);
      }
    catch (const ::CCM_DDS::InternalError &ex)
      {
        ACE_ERROR ((LM_ERROR, "Receiver_exec_i::set_filter - "
                    "ERROR: Unexpected InternalError exception caught "
                    "with <%C> as error\n.",
                    ::CIAO::DDS4CCM::translate_retcode (ex.error_code)));
      }
    catch (...)
      {
        ACE_ERROR ((LM_ERROR, "Receiver_exec_i::set_filter - "
                    "ERROR: Unexpected exception caught.\n"));
      }
  }

  void
  Receiver_exec_i::start_read (::CORBA::UShort run)
  {
    this->ticker_ = new read_action_Generator (*this, run);
    if (this->reactor ()->schedule_timer (this->ticker_,
                                          0,
                                          ACE_Time_Value(1, 0),
                                          ACE_Time_Value(1, 0)) == -1)
      {
        ACE_ERROR ((LM_ERROR, "Unable to schedule Timer\n"));
      }
  }

  void
  Receiver_exec_i::run (::CORBA::UShort run)
  {
    if (this->ticker_)
      {
        this->reactor ()->cancel_timer (this->ticker_);
        delete this->ticker_;
        this->ticker_ = 0;
      }
    ACE_DEBUG ((LM_DEBUG, "Receiver_exec_i::run - "
                          "Starting run number <%d>\n",
                          run));
    TwoQueriesRestarter_var restarter =
      this->ciao_context_->get_connection_writer_restart ();
    if (!::CORBA::is_nil (restarter.in ()))
      {
        switch (run)
        {
          case 1:
            {
              this->samples_received_ += this->test_all (run);
              this->check_filter (run);
              this->read_all (run);
              //set filter for the next run
              this->set_filter (run + 1);
              // inform the sender that it may start the next run
              restarter->restart_write ();
            }
            break;
          case 2:
            {
              this->samples_received_ += this->test_all (run);
              check_filter (run);
              this->read_all (run);
              //set filter for the next run
              this->set_filter (run + 1);
              // inform the sender that it may start the next run
              restarter->restart_write ();
            }
            break;
          case 3:
            {
              this->samples_received_ += this->test_all (run);
              this->check_filter (run);
              this->read_all (run);
              //set filter for the next run
              this->set_filter (run + 1);
              // inform the sender that it may start the next run
              restarter->restart_write ();
            }
            break;
          case 4:
            {
              this->samples_received_ += this->test_all (run);
              this->read_all (run);
            }
        }
      }
    else
      {
        ACE_ERROR ((LM_ERROR, "Receiver_exec_i::run - "
                  "ERROR: Reference to Restarter not found\n"));
      }
  }

  void
  Receiver_exec_i::iterations (::CORBA::UShort iterations)
  {
    this->iterations_ = iterations;
  }

  void
  Receiver_exec_i::keys (::CORBA::UShort keys)
  {
    this->keys_ = keys;

    //the last key is used for checking if all samples are received and therefore
    //get access status 1. So these samples are not read by following
    //get_all method.
    this->samples_expected_ = (this->keys_ - 1) * SAMPLES_PER_KEY_GETTER;
  }

  // Component attributes and port operations.

  ::CCM_DDS::CCM_PortStatusListener_ptr
  Receiver_exec_i::get_get_port_status (void)
  {
    if ( ::CORBA::is_nil (this->ciao_get_port_status_.in ()))
      {
        get_port_status_exec_i *tmp = 0;
        ACE_NEW_RETURN (
          tmp,
          get_port_status_exec_i (
            this->ciao_context_.in ()),
            ::CCM_DDS::CCM_PortStatusListener::_nil ());

          this->ciao_get_port_status_ = tmp;
      }

    return
      ::CCM_DDS::CCM_PortStatusListener::_duplicate (
        this->ciao_get_port_status_.in ());
  }

  ::CCM_DDS::CCM_PortStatusListener_ptr
  Receiver_exec_i::get_read_port_status (void)
  {
    return ::CCM_DDS::CCM_PortStatusListener::_nil ();
  }

  ::CCM_TwoQueriesStarter_ptr
  Receiver_exec_i::get_reader_start (void)
  {
    if ( ::CORBA::is_nil (this->ciao_reader_start_.in ()))
      {
        reader_start_exec_i *tmp = 0;
        ACE_NEW_RETURN (
          tmp,
          reader_start_exec_i (
            this->ciao_context_.in (),
            *this),
            ::CCM_TwoQueriesStarter::_nil ());

          this->ciao_reader_start_ = tmp;
      }

    return
      ::CCM_TwoQueriesStarter::_duplicate (
        this->ciao_reader_start_.in ());
  }

  // Operations from Components::SessionComponent.

  void
  Receiver_exec_i::set_session_context (
    ::Components::SessionContext_ptr ctx)
  {
    this->ciao_context_ =
      ::QCTQM_Test::CCM_Receiver_Context::_narrow (ctx);

    if ( ::CORBA::is_nil (this->ciao_context_.in ()))
      {
        throw ::CORBA::INTERNAL ();
      }
  }

  void
  Receiver_exec_i::configuration_complete (void)
  {
    /* Your code here. */
  }

  void
  Receiver_exec_i::ccm_activate (void)
  {
    ::QCTQM_Test::QueryConditionTestConnector::Getter_var getter =
      this->ciao_context_->get_connection_get_port_fresh_data ();
    DDS::Duration_t to;
    to.sec = 5; to.nanosec = 0;
    getter->time_out (to);
    //set filter for the first run
    this->set_filter (1);
  }

  void
  Receiver_exec_i::ccm_passivate (void)
  {
    if (this->ticker_)
      {
        this->reactor ()->cancel_timer (this->ticker_);
        delete this->ticker_;
        this->ticker_ = 0;
      }
  }

  void
  Receiver_exec_i::ccm_remove (void)
  {
    if (this->samples_received_ != this->samples_expected_)
      {
        ACE_ERROR ((LM_ERROR, "ERROR: TWO QUERIES GET_MANY : "
                              "Unexpected number of samples received: "
                              "expected <%d> - received <%d>\n",
                              this->samples_expected_,
                              this->samples_received_));
      }
    else
      {
        ACE_DEBUG ((LM_DEBUG, "TWO QUERIES : GET_MANY : "
                              "Expected number of samples received: "
                              "expected <%d> - received <%d>\n",
                              this->samples_expected_,
                              this->samples_received_));
      }
  }

  extern "C" RECEIVER_EXEC_Export ::Components::EnterpriseComponent_ptr
  create_QCTQM_Test_Receiver_Impl (void)
  {
    ::Components::EnterpriseComponent_ptr retval =
      ::Components::EnterpriseComponent::_nil ();

    ACE_NEW_NORETURN (
      retval,
      Receiver_exec_i);

    return retval;
  }
}