/*******************************************************************************
 * BSD 3-Clause License
 *
 * Copyright (c) 2016-2025, Mech-Mind Robotics Technologies Co., Ltd.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 *   list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *   this list of conditions and the following disclaimer in the documentation
 *   and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its
 *   contributors may be used to endorse or promote products derived from
 *   this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * Info:  https://www.mech-mind.com/
 *
 ******************************************************************************/

#pragma once
#include <functional>
#include "profiler/Profiler.h"

namespace mmind {

namespace eye {

/**
 * @brief Describes the event of profiler.
 * Use @ref ProfilerEvent.registerProfilerEventCallback to register an event of interest.
 */
class MMIND_API_EXPORT ProfilerEvent
{
public:
    /**
     * @brief Describes the types of Event.
     * @deprecated This enum is deprecated since version 2.5.0. Please use the following method:
     * @ref getSupportedEvents(const Profiler&, std::vector<EventInfo>&) to get supported events.
     */
    enum Event {
        PROFILER_EVENT_DISCONNECTED = 0x0001, ///< The profiler has disconnected.
        PROFILER_EVENT_ALL = 0xFFFF,          ///< All profiler events.
    };

    using EventCallback = std::function<void(Event event, void* pUser)>;

    /**
     * @brief Registers a profiler event of interest.
     * @deprecated This method is deprecated since version 2.5.0. Please use the following method:
     * @ref registerProfilerEventCallback(Profiler&, uint16_t, const ProfilerEventCallback&).
     * @param [in] profiler The profiler handle.
     * @param [in] callback The callback function for responding to profiler events.
     * @param [in] pUser Pointer used by the user.
     * @param [in] events The profiler event. See @ref ProfilerEvent.Event for details.
     * @return
     *  @ref ErrorStatus.MMIND_STATUS_SUCCESS Success.\n
     *  @ref ErrorStatus.MMIND_STATUS_INVALID_DEVICE Invalid profiler handle.\n
     *  @ref ErrorStatus.MMIND_STATUS_DEVICE_OFFLINE Profiler disconnected.\n
     *  @ref ErrorStatus.MMIND_STATUS_NO_SUPPORT_ERROR Not supported profiler model or firmware
     *  version.\n
     */
    [[deprecated(
        "Please use @ref registerProfilerEventCallback(Profiler& profiler, Event event, const "
        "ProfilerEventCallback& callback) instead.")]] static ErrorStatus
    registerProfilerEventCallback(Profiler& profiler, EventCallback callback, void* pUser,
                                  unsigned int events);

    /**
     * @brief Describes the event information.
     */
    struct EventInfo
    {
        std::string eventName{};  ///< The name of the event.
        uint16_t eventId{0x0000}; ///< The ID of the event.

        EventInfo() = default;
        EventInfo(const std::string& eventName, uint16_t eventId)
            : eventName(eventName), eventId(eventId)
        {
        }
    };

    /**
     * @brief Get supported events by the profiler
     *
     * @param profiler The profiler for which to get supported events.
     * @param eventInfos The information of the events supported.
     * @return
     *  @ref ErrorStatus.MMIND_STATUS_SUCCESS Success.\n
     *  @ref ErrorStatus.MMIND_STATUS_INVALID_DEVICE Invalid camera handle.\n
     *  @ref ErrorStatus.MMIND_STATUS_DEVICE_OFFLINE Camera disconnected.\n
     */
    static ErrorStatus getSupportedEvents(const Profiler& profiler,
                                          std::vector<EventInfo>& eventInfos);

    struct EventData
    {
        int eventId{0};          ///< Event ID, defined in @ref ProfilerEvent::Event.
        std::string eventName{}; ///< Event name.
        int64_t timestamp{0};    ///< The timestamp of the event occurrence.
    };

    /**
     * @brief Describes the data member of the event's payload
     */
    struct PayloadMember
    {
        std::string name; ///< Member name
        enum class Type {
            _UInt32,
            _Int32,
            _Int64,
            _Float,
            _Double,
            _Bool,
            _String,
        };
        Type type; ///< Member type
        struct MemberValue
        {
            uint32_t uint32Value;
            int32_t int32Value;
            int64_t int64Value;
            float floatValue;
            double doubleValue;
            bool boolValue;
            std::string stringValue;
        };
        MemberValue value; ///< Member value
    };

    using Payload = std::vector<PayloadMember>;

    /**
     * @brief callback function for a camera event.
     * @param [in] eventData shared data among events.
     * @param [in] extraPayload event-specific data.
     */
    using ProfilerEventCallback =
        std::function<void(const EventData* eventData, const void* extraPayload)>;

    /**
     * @brief Registers a callback function to be executed when the specified @ref Event occurs on
     the specified @ref Profiler object.
     * @param profiler The profiler for which the callback function is registered.
     * @param event The event for which the callback function is registered.
     * @param callback The callback function to be executed when the specified event occurs.

     * @return
     *  @ref ErrorStatus.MMIND_STATUS_SUCCESS Success.\n
     *  @ref ErrorStatus.MMIND_STATUS_INVALID_DEVICE Invalid profiler handle.\n
     *  @ref ErrorStatus.MMIND_STATUS_DEVICE_OFFLINE Profiler disconnected.\n
     *  @ref ErrorStatus.MMIND_STATUS_NO_SUPPORT_ERROR Unsupported profiler model or firmware
     version.\n
     *  @ref ErrorStatus.MMIND_STATUS_INVALID_CALLBACKFUNC The registered callback function is
     invalid.\n
     * @ref ErrorStatus.MMIND_STATUS_DUPLICATED_REGISTRATION A callback function for this event has
     already been registered.\n
     *  @ref ErrorStatus.MMIND_STATUS_MESSAGE_CHANNEL_ERROR Failed to establish the channel for
     delivering the event occurrence.\n
     */
    static ErrorStatus registerProfilerEventCallback(Profiler& profiler, uint16_t event,
                                                     const ProfilerEventCallback& callback);

    /**
       * @brief Cancels the registration of a callback function for the specified event and @ref
       Profiler object.
       * @param profiler The Profiler for which to cancel the registration of the callback function.
       * @param event The event for which to cancel the registration of the callback function.

       * @return
       *  @ref ErrorStatus.MMIND_STATUS_SUCCESS Success.\n
       *  @ref ErrorStatus.MMIND_STATUS_INVALID_DEVICE Invalid profiler handle.\n
       *  @ref ErrorStatus.MMIND_STATUS_DEVICE_OFFLINE Profiler disconnected.\n
       *  @ref ErrorStatus.MMIND_STATUS_NO_SUPPORT_ERROR Unsupported profiler model or firmware
       version.\n
       *  @ref ErrorStatus.MMIND_STATUS_INVALID_CALLBACKFUNC The registered callback function is
       invalid.\n
       *  @ref ErrorStatus.MMIND_STATUS_MESSAGE_CHANNEL_ERROR Failed to close the channel for
       delivering the event occurrence.\n
       */
    static ErrorStatus unregisterProfilerEventCallback(Profiler& profiler, uint16_t event);
};

} // namespace eye

} // namespace mmind
