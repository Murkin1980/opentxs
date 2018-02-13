/************************************************************
 *
 *                 OPEN TRANSACTIONS
 *
 *       Financial Cryptography and Digital Cash
 *       Library, Protocol, API, Server, CLI, GUI
 *
 *       -- Anonymous Numbered Accounts.
 *       -- Untraceable Digital Cash.
 *       -- Triple-Signed Receipts.
 *       -- Cheques, Vouchers, Transfers, Inboxes.
 *       -- Basket Currencies, Markets, Payment Plans.
 *       -- Signed, XML, Ricardian-style Contracts.
 *       -- Scripted smart contracts.
 *
 *  EMAIL:
 *  fellowtraveler@opentransactions.org
 *
 *  WEBSITE:
 *  http://www.opentransactions.org/
 *
 *  -----------------------------------------------------
 *
 *   LICENSE:
 *   This Source Code Form is subject to the terms of the
 *   Mozilla Public License, v. 2.0. If a copy of the MPL
 *   was not distributed with this file, You can obtain one
 *   at http://mozilla.org/MPL/2.0/.
 *
 *   DISCLAIMER:
 *   This program is distributed in the hope that it will
 *   be useful, but WITHOUT ANY WARRANTY; without even the
 *   implied warranty of MERCHANTABILITY or FITNESS FOR A
 *   PARTICULAR PURPOSE.  See the Mozilla Public License
 *   for more details.
 *
 ************************************************************/

#ifndef OPENTXS_API_CLIENT_SYNC_IMPLEMENTATION_HPP
#define OPENTXS_API_CLIENT_SYNC_IMPLEMENTATION_HPP

#include "opentxs/Internal.hpp"

#include "opentxs/api/client/Sync.hpp"
#include "opentxs/core/Lockable.hpp"
#include "opentxs/core/UniqueQueue.hpp"

#include <atomic>
#include <memory>
#include <map>
#include <thread>
#include <tuple>

namespace opentxs
{
namespace api
{
namespace client
{
namespace implementation
{

class Sync : virtual public client::Sync, Lockable
{
public:
    bool AcceptIncoming(
        const Identifier& nymID,
        const Identifier& accountID,
        const Identifier& serverID,
        const std::size_t max = DEFAULT_PROCESS_INBOX_ITEMS) const override;
    Messagability CanMessage(
        const Identifier& senderNymID,
        const Identifier& recipientContactID) const override;
    Identifier FindNym(const Identifier& nymID) const override;
    Identifier FindNym(const Identifier& nymID, const Identifier& serverIDHint)
        const override;
    Identifier FindServer(const Identifier& serverID) const override;
    const Identifier& IntroductionServer() const override;
    Identifier MessageContact(
        const Identifier& senderNymID,
        const Identifier& contactID,
        const std::string& message) const override;
    void Refresh() const override;
    std::uint64_t RefreshCount() const override;
    Identifier RegisterNym(
        const Identifier& nymID,
        const Identifier& server,
        const bool setContactData) const override;
    Identifier SetIntroductionServer(
        const ServerContract& contract) const override;
    Identifier ScheduleDownloadAccount(
        const Identifier& localNymID,
        const Identifier& serverID,
        const Identifier& accountID) const override;
    Identifier ScheduleDownloadContract(
        const Identifier& localNymID,
        const Identifier& serverID,
        const Identifier& contractID) const override;
    Identifier ScheduleDownloadNym(
        const Identifier& localNymID,
        const Identifier& serverID,
        const Identifier& targetNymID) const override;
    Identifier ScheduleDownloadNymbox(
        const Identifier& localNymID,
        const Identifier& serverID) const override;
    Identifier ScheduleRegisterNym(
        const Identifier& localNymID,
        const Identifier& serverID) const override;
    void StartIntroductionServer(const Identifier& localNymID) const override;
    ThreadStatus Status(const Identifier& taskID) const override;

    ~Sync();

private:
    static const std::string DEFAULT_INTRODUCTION_SERVER;

    friend api::implementation::Api;

    /** ContextID: localNymID, serverID */
    using ContextID = std::pair<Identifier, Identifier>;
    /** MessageTask: recipientID, message<*/
    using MessageTask = std::pair<Identifier, std::string>;

    struct OperationQueue {
        UniqueQueue<Identifier> check_nym_;
        UniqueQueue<bool> register_nym_;
        UniqueQueue<Identifier> download_account_;
        UniqueQueue<Identifier> download_contract_;
        UniqueQueue<bool> download_nymbox_;
        UniqueQueue<MessageTask> send_message_;
    };

    std::recursive_mutex& api_lock_;
    const std::atomic<bool>& shutdown_;
    const OT_API& ot_api_;
    const opentxs::OTAPI_Exec& exec_;
    const api::ContactManager& contacts_;
    const api::Settings& config_;
    const api::client::ServerAction& server_action_;
    const api::client::Wallet& wallet_;
    const api::crypto::Encode& encoding_;
    mutable std::mutex introduction_server_lock_{};
    mutable std::mutex nym_fetch_lock_{};
    mutable std::mutex task_status_lock_{};
    mutable std::atomic<std::uint64_t> refresh_counter_{0};
    mutable std::map<ContextID, OperationQueue> operations_{};
    mutable std::map<Identifier, UniqueQueue<Identifier>> server_nym_fetch_{};
    UniqueQueue<Identifier> missing_nyms_{};
    UniqueQueue<Identifier> missing_servers_{};
    mutable std::map<ContextID, std::unique_ptr<std::thread>> state_machines_{};
    mutable std::unique_ptr<Identifier> introduction_server_id_{nullptr};
    mutable std::map<Identifier, ThreadStatus> task_status_{};

    std::pair<bool, std::size_t> accept_incoming(
        const rLock& lock,
        const std::size_t max,
        const Identifier& accountID,
        ServerContext& context) const;
    void add_task(const Identifier& taskID, const ThreadStatus status) const;
    Messagability can_message(
        const Identifier& senderID,
        const Identifier& recipientID,
        Identifier& recipientNymID,
        Identifier& serverID) const;
    void check_nym_revision(const ServerContext& context, OperationQueue& queue)
        const;
    bool check_registration(
        const Identifier& nymID,
        const Identifier& serverID,
        std::shared_ptr<const ServerContext>& context) const;
    bool check_server_contract(const Identifier& serverID) const;
    bool download_account(
        const Identifier& taskID,
        const Identifier& nymID,
        const Identifier& serverID,
        const Identifier& accountID) const;
    bool download_contract(
        const Identifier& taskID,
        const Identifier& nymID,
        const Identifier& serverID,
        const Identifier& contractID) const;
    bool download_nym(
        const Identifier& taskID,
        const Identifier& nymID,
        const Identifier& serverID,
        const Identifier& targetNymID) const;
    bool download_nymbox(
        const Identifier& taskID,
        const Identifier& nymID,
        const Identifier& serverID) const;
    bool find_nym(
        const Identifier& nymID,
        const Identifier& serverID,
        const Identifier& targetServerID) const;
    bool find_server(
        const Identifier& nymID,
        const Identifier& serverID,
        const Identifier& targetNymID) const;
    bool finish_task(const Identifier& taskID, const bool success) const;
    bool get_admin(
        const Identifier& nymID,
        const Identifier& serverID,
        const OTPassword& password) const;
    Identifier get_introduction_server(const Lock& lock) const;
    UniqueQueue<Identifier>& get_nym_fetch(const Identifier& serverID) const;
    OperationQueue& get_operations(const ContextID& id) const;
    Identifier import_default_introduction_server(const Lock& lock) const;
    void load_introduction_server(const Lock& lock) const;
    bool message_nym(
        const Identifier& taskID,
        const Identifier& nymID,
        const Identifier& serverID,
        const Identifier& targetNymID,
        const std::string& text) const;
    bool publish_server_registration(
        const Identifier& nymID,
        const Identifier& serverID,
        const bool forcePrimary) const;
    Identifier random_id() const;
    void refresh_accounts() const;
    void refresh_contacts() const;
    bool register_nym(
        const Identifier& taskID,
        const Identifier& nymID,
        const Identifier& serverID) const;
    Identifier schedule_download_nymbox(
        const Identifier& localNymID,
        const Identifier& serverID) const;
    Identifier set_introduction_server(
        const Lock& lock,
        const ServerContract& contract) const;
    Identifier start_task(const Identifier& taskID, bool success) const;
    void state_machine(const ContextID id, OperationQueue& queue) const;
    void update_task(const Identifier& taskID, const ThreadStatus status) const;
    void start_introduction_server(const Identifier& nymID) const;

    Sync(
        std::recursive_mutex& apiLock,
        const std::atomic<bool>& shutdown,
        const OT_API& otapi,
        const opentxs::OTAPI_Exec& exec,
        const api::ContactManager& contacts,
        const api::Settings& config,
        const api::client::ServerAction& serverAction,
        const api::client::Wallet& wallet,
        const api::crypto::Encode& encoding);
    Sync() = delete;
    Sync(const Sync&) = delete;
    Sync(Sync&&) = delete;
    Sync& operator=(const Sync&) = delete;
    Sync& operator=(Sync&&) = delete;
};
}  // namespace implementation
}  // namespace client
}  // namespace api
}  // namespace opentxs
#endif  // OPENTXS_API_CLIENT_SYNC_IMPLEMENTATION_HPP
