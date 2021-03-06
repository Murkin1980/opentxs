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

#include "opentxs/stdafx.hpp"

#include "UI.hpp"

#include "ui/ContactList.hpp"

//#define OT_METHOD "opentxs::api::implementation::UI"

namespace opentxs::api::implementation
{
UI::UI(
    const opentxs::network::zeromq::Context& zmq,
    const api::ContactManager& contact)
    : zmq_(zmq)
    , contact_(contact)
    , contact_lists_()
{
}

const ui::ContactList& UI::ContactList(const Identifier& nymID) const
{
    Lock lock(lock_);

    auto& list = contact_lists_[nymID];

    if (false == bool(list)) {
        list.reset(new ui::implementation::ContactList(zmq_, contact_, nymID));
    }

    OT_ASSERT(list)

    return *list;
}

UI::~UI() {}
}  // namespace opentxs::api::implementation
