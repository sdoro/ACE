// $Id: SHS_Interceptor.cpp 92665 2010-11-22 10:23:33Z johnnyw $

#include "SHS_Interceptor.h"
#include "dance/Deployment/Deployment_StartErrorC.h"
#include "dance/DAnCE_PropertiesC.h"
#include "dance/DAnCE_Utility.h"
#include "dance/Logger/Log_Macros.h"
#include "System_Health/SHS_DataModelC.h"
#include "System_Health/Interceptors/SHS_CORBA_Transport.h"

namespace DAnCE
{
  // Implementation skeleton constructor
  SHS_Interceptor::SHS_Interceptor (void)
  {
  }

  // Implementation skeleton destructor
  SHS_Interceptor::~SHS_Interceptor (void)
  {
  }

  void
  SHS_Interceptor::configure (const ::Deployment::Properties &props )
  {
    DAnCE::SHS::SHS_Transport *tmp (0);

    ACE_NEW_THROW_EX (tmp,
                      SHS::SHS_CORBA_Transport (),
                      CORBA::NO_MEMORY ());

    this->shs_transport_.reset (tmp);

    this->shs_transport_->configure (props);
  }

  void
  SHS_Interceptor::post_install (const ::Deployment::DeploymentPlan &plan,
                                 ::CORBA::ULong index,
                                 const ::CORBA::Any &reference,
                                 const ::CORBA::Any &exception)
  {
    ::DAnCE::SHS::Status_Update update;
    CORBA::ULong mdd_idx = plan.instance[index].implementationRef;

    update.id = plan.instance[index].name.in ();
    update.type =
      DAnCE::Utility::get_instance_type (plan.implementation[mdd_idx].execParameter);
    CORBA::ULong idx (0);

    if (exception.type() != ::CORBA::_tc_null)
      {
        std::string result;
        DAnCE::Utility::stringify_exception_from_any (exception,
                                                      result);
        update.new_status = DAnCE::SHS::INST_ERROR;
        update.instance_info.length (idx + 1);
        update.instance_info[idx].name = ::DAnCE::SHS::Constants::SHS_DIAGNOSTIC;
        update.instance_info[idx].value <<= CORBA::Any::from_string (result.c_str (), 0);

        ++idx;
      }
    else
      {
        update.new_status = DAnCE::SHS::INST_INSTALLED;
      }

    update.instance_info.length (idx + 1);
    update.instance_info[idx].name = ::DAnCE::SHS::Constants::SHS_INSTANCE_REF;
    update.instance_info[idx].value = reference;

    if (this->shs_transport_.get ())
      this->shs_transport_->push_event (update);
  }

  void
  SHS_Interceptor::post_connect (const ::Deployment::DeploymentPlan &plan,
                                 ::CORBA::ULong connection,
                                 const ::CORBA::Any &exception)
  {
    ::DAnCE::SHS::Status_Update update;
    const ::Deployment::PlanConnectionDescription &pcd = plan.connection[connection];
    update.id = pcd.name.in ();
    update.type = DAnCE::SHS::Constants::SHS_CONNECTION;
    CORBA::ULong info_idx (0);

    if (exception.type() != ::CORBA::_tc_null)
      {
        std::string result;
        DAnCE::Utility::stringify_exception_from_any (exception, result);

        update.new_status = DAnCE::SHS::INST_ERROR;
        update.instance_info.length (info_idx + 1);
        update.instance_info[info_idx].name = ::DAnCE::SHS::Constants::SHS_DIAGNOSTIC;
        update.instance_info[info_idx].value <<= CORBA::Any::from_string (result.c_str (), 0);
        ++info_idx;
      }
    else
      {
        update.new_status = DAnCE::SHS::INST_ACTIVE;
      }

    for (CORBA::ULong i = 0; i < pcd.internalEndpoint.length (); ++i)
      {
        std::string id (plan.instance[pcd.internalEndpoint[i].instanceRef].name.in ());
        id += ':';
        id += pcd.internalEndpoint[i].portName.in ();

        update.instance_info.length (info_idx + 1);

        if (pcd.internalEndpoint[i].provider)
          {
            update.instance_info[info_idx].name = DAnCE::SHS::Constants::SHS_CONN_PROVIDER;
          }
        else
          {
            update.instance_info[info_idx].name = DAnCE::SHS::Constants::SHS_CONN_RECIPIENT;
          }

        update.instance_info[info_idx].value <<= CORBA::Any::from_string (id.c_str (), 0);
        ++info_idx;
      }

    for (CORBA::ULong i = 0; i < pcd.externalEndpoint.length (); ++i)
      {
        update.instance_info.length (info_idx + 1);
        update.instance_info[info_idx].name = DAnCE::SHS::Constants::SHS_CONN_EXTERNAL_ENDPOINT;
        update.instance_info[info_idx].value <<= pcd.externalEndpoint[i];
        ++info_idx;
      }

    if (this->shs_transport_.get ())
      this->shs_transport_->push_event (update);
  }

  void
  SHS_Interceptor::post_configured (const ::Deployment::DeploymentPlan & plan,
                                    ::CORBA::ULong index,
                                    const ::CORBA::Any &exception )
  {
    ::DAnCE::SHS::Status_Update update;
    CORBA::ULong mdd_idx = plan.instance[index].implementationRef;

    update.id = plan.instance[index].name.in ();
    update.type =
      DAnCE::Utility::get_instance_type (plan.implementation[mdd_idx].execParameter);

    if (exception.type() != ::CORBA::_tc_null)
      {
        std::string result;
        DAnCE::Utility::stringify_exception_from_any (exception,
                                                      result);

        update.new_status = DAnCE::SHS::INST_ERROR;
        update.instance_info.length (1);
        update.instance_info[0].name = ::DAnCE::SHS::Constants::SHS_DIAGNOSTIC;
        update.instance_info[0].value <<= CORBA::Any::from_string (result.c_str (), 0);
      }
    else
      {
        update.new_status = DAnCE::SHS::INST_PASSIVE;
      }

    if (this->shs_transport_.get ())
      this->shs_transport_->push_event (update);
  }


  void
  SHS_Interceptor::post_activate (const ::Deployment::DeploymentPlan & plan,
                                  ::CORBA::ULong index,
                                  const ::CORBA::Any & exception)
  {
    ::DAnCE::SHS::Status_Update update;
    CORBA::ULong mdd_idx = plan.instance[index].implementationRef;

    update.id = plan.instance[index].name.in ();
    update.type =
      DAnCE::Utility::get_instance_type (plan.implementation[mdd_idx].execParameter);

    if (exception.type() != ::CORBA::_tc_null)
      {
        std::string result;
        DAnCE::Utility::stringify_exception_from_any (exception,
                                                      result);

        update.new_status = DAnCE::SHS::INST_ERROR;
        update.instance_info.length (1);
        update.instance_info[0].name = ::DAnCE::SHS::Constants::SHS_DIAGNOSTIC;
        update.instance_info[0].value <<= CORBA::Any::from_string (result.c_str (), 0);
      }
    else
      {
        update.new_status = DAnCE::SHS::INST_ACTIVE;
      }

    if (this->shs_transport_.get ())
      this->shs_transport_->push_event (update);
  }


  void
  SHS_Interceptor::post_passivate (const ::Deployment::DeploymentPlan & plan,
                                   ::CORBA::ULong index,
                                   const ::CORBA::Any & exception)
  {
    ::DAnCE::SHS::Status_Update update;
    CORBA::ULong mdd_idx = plan.instance[index].implementationRef;

    update.id = plan.instance[index].name.in ();
    update.type =
      DAnCE::Utility::get_instance_type (plan.implementation[mdd_idx].execParameter);

    if (exception.type() != ::CORBA::_tc_null)
      {
        std::string result;
        DAnCE::Utility::stringify_exception_from_any (exception,
                                                      result);

        update.new_status = DAnCE::SHS::INST_ERROR;
        update.instance_info.length (1);
        update.instance_info[0].name = ::DAnCE::SHS::Constants::SHS_DIAGNOSTIC;
        update.instance_info[0].value <<= CORBA::Any::from_string (result.c_str (), 0);
      }
    else
      {
        update.new_status = DAnCE::SHS::INST_PASSIVE;
      }

    if (this->shs_transport_.get ())
      this->shs_transport_->push_event (update);
  }


  void
  SHS_Interceptor::post_remove (const ::Deployment::DeploymentPlan & plan,
                                ::CORBA::ULong index,
                                const ::CORBA::Any & exception)
  {
    ::DAnCE::SHS::Status_Update update;
    CORBA::ULong mdd_idx = plan.instance[index].implementationRef;

    update.id = plan.instance[index].name.in ();
    update.type =
      DAnCE::Utility::get_instance_type (plan.implementation[mdd_idx].execParameter);

    if (exception.type() != ::CORBA::_tc_null)
      {
        std::string result;
        DAnCE::Utility::stringify_exception_from_any (exception,
                                                      result);

        update.new_status = DAnCE::SHS::INST_ERROR;
        update.instance_info.length (1);
        update.instance_info[0].name = ::DAnCE::SHS::Constants::SHS_DIAGNOSTIC;
        update.instance_info[0].value <<= CORBA::Any::from_string (result.c_str (), 0);
      }
    else
      {
        update.new_status = DAnCE::SHS::INST_DEFUNCT;
      }

    if (this->shs_transport_.get ())
      this->shs_transport_->push_event (update);
  }

  void
  SHS_Interceptor::unexpected_event (const ::Deployment::DeploymentPlan & plan,
                                     ::CORBA::ULong index,
                                     const ::CORBA::Any & exception,
                                     const char *error)
  {
    ::DAnCE::SHS::Status_Update update;
    CORBA::ULong mdd_idx = plan.instance[index].implementationRef;

    update.id = plan.instance[index].name.in ();
    update.type =
      DAnCE::Utility::get_instance_type (plan.implementation[mdd_idx].execParameter);

    update.new_status = DAnCE::SHS::INST_ERROR;
    CORBA::ULong pos (0);

    if (exception.type() != ::CORBA::_tc_null)
      {
        std::string result;
        DAnCE::Utility::stringify_exception_from_any (exception,
                                                      result);

        update.instance_info.length (pos + 1);
        update.instance_info[pos].name = ::DAnCE::SHS::Constants::SHS_DIAGNOSTIC;
        update.instance_info[pos].value <<= CORBA::Any::from_string (result.c_str (), 0);
        ++pos;
      }

    if (error)
      {
        update.instance_info.length (pos + 1);
        update.instance_info[pos].name = ::DAnCE::SHS::Constants::SHS_DIAGNOSTIC;
        update.instance_info[pos].value <<= CORBA::Any::from_string (error, 0);
        ++pos;
      }

    if (this->shs_transport_.get ())
      this->shs_transport_->push_event (update);
  }
}

extern "C"
{
  ::DAnCE::DeploymentInterceptor_ptr
  create_DAnCE_SHS_Interceptor (void)
  {
    return new DAnCE::SHS_Interceptor ();
  }
}
