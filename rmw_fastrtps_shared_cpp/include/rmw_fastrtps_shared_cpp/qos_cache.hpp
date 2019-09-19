// Copyright 2018 Amazon.com, Inc. or its affiliates. All Rights Reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef RMW_FASTRTPS_SHARED_CPP_QOS_CACHE_HPP
#define RMW_FASTRTPS_SHARED_CPP_QOS_CACHE_HPP

#include "fastrtps/rtps/common/InstaneHandle.h"

typedef eprosima::fastrtps::rtps::GUID_t GUID_t;
/**
 * Qos cache data structure. Manages relationship between participants and their qos policies.
 */
class QosCache
{
private:
  typedef std::map<GUID_t, rmw_qos_profile_t> ParticipantQosMap;

  /**
   * Map of participant guid to its respective qos policy.
   */
  ParticipantQosMap participant_to_qos_;

  void initializeQosForParticipant(const GUID_t & guid, ParticipantQosMap & participant_qos_map)
  {
    if (participant_qos_map.find(guid) == participant_qos_map.end()) {
      participant_qos_map[guid] = rmw_qos_profile_t();
    }
  }

  template<typename DDSQoSPolicy>
  void
  dds_qos_policy_to_rmw(
      const DDSQoSPolicy & dds_qos,
      rmw_qos_profile_t * qos)
  {
    switch (dds_qos.m_reliability.kind) {
      case eprosima::fastrtps::BEST_EFFORT_RELIABILITY_QOS:
        qos->reliability = RMW_QOS_POLICY_RELIABILITY_BEST_EFFORT;
        break;
      case eprosima::fastrtps::RELIABLE_RELIABILITY_QOS:
        qos->reliability = RMW_QOS_POLICY_RELIABILITY_RELIABLE;
        break;
      default:
        qos->reliability = RMW_QOS_POLICY_RELIABILITY_UNKNOWN;
        break;
    }

    switch (dds_qos.m_durability.kind) {
      case eprosima::fastrtps::TRANSIENT_LOCAL_DURABILITY_QOS:
        qos->durability = RMW_QOS_POLICY_DURABILITY_TRANSIENT_LOCAL;
        break;
      case eprosima::fastrtps::VOLATILE_DURABILITY_QOS:
        qos->durability = RMW_QOS_POLICY_DURABILITY_VOLATILE;
        break;
      default:
        qos->durability = RMW_QOS_POLICY_DURABILITY_UNKNOWN;
        break;
    }

    qos->deadline.sec = dds_qos.m_deadline.period.seconds;
    qos->deadline.nsec = dds_qos.m_deadline.period.nanosec;

    qos->lifespan.sec = dds_qos.m_lifespan.duration.seconds;
    qos->lifespan.nsec = dds_qos.m_lifespan.duration.nanosec;

    switch (dds_qos.m_liveliness.kind) {
      case eprosima::fastrtps::AUTOMATIC_LIVELINESS_QOS:
        qos->liveliness = RMW_QOS_POLICY_LIVELINESS_AUTOMATIC;
        break;
      case eprosima::fastrtps::MANUAL_BY_PARTICIPANT_LIVELINESS_QOS:
        qos->liveliness = RMW_QOS_POLICY_LIVELINESS_MANUAL_BY_NODE;
        break;
      case eprosima::fastrtps::MANUAL_BY_TOPIC_LIVELINESS_QOS:
        qos->liveliness = RMW_QOS_POLICY_LIVELINESS_MANUAL_BY_TOPIC;
        break;
      default:
        qos->liveliness = RMW_QOS_POLICY_LIVELINESS_UNKNOWN;
        break;
    }
    qos->liveliness_lease_duration.sec = dds_qos.m_liveliness.lease_duration.seconds;
    qos->liveliness_lease_duration.nsec = dds_qos.m_liveliness.lease_duration.nanosec;
  }

public:

  const ParticipantQosMap & getParticipantQosMap() const
  {
    return participant_to_qos_;
  }

  template<class T>
  bool addQosPolicy(
      const eprosima::fastrtps::rtps::InstanceHandle_t & rtpsParticipantKey,
      const T & dds_qos)
  {
    auto guid = iHandle2GUID(rtpsParticipantKey);
    initializeQosForParticipant(guid, participant_to_qos_);
    //TODO add logger code
    //convert dds_qos to rmw_qos_profile_t
    auto qos_profile = rmw_qos_profile_t();
    dds_qos_policy_to_rmw(dds_qos, qos_profile);
    //add rmw_qos_profile to map
    participant_to_qos_[guid] = qos_profile;
    return true
  }

  bool removeQosPolicy(const eprosima::fastrtps::rtps::InstanceHandle_t & rtpsParticipantKey)
  {
    auto guid = iHandle2GUID(rtpsParticipantKey);
    if (participant_to_qos_.find(guid) == participant_to_qos_.end()) {
      //TODO LOG
      return false;
    }
    participant_to_qos_.erase(guid);
    return;
  }
};

#endif //RMW_FASTRTPS_SHARED_CPP_QOS_CACHE_HPP
