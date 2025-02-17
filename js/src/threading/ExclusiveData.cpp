/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 4 -*-
 * vim: set ts=8 sts=4 et sw=4 tw=99:
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "threading/ExclusiveData.h"

namespace js {

/* static */ mozilla::Maybe<detail::ExclusiveDataBase>
detail::ExclusiveDataBase::Create()
{
    auto lock = PR_NewLock();
    if (!lock)
        return mozilla::Nothing();

    return mozilla::Some(detail::ExclusiveDataBase(lock));
}

detail::ExclusiveDataBase::~ExclusiveDataBase()
{
    if (lock_)
        PR_DestroyLock(lock_);
}

void
detail::ExclusiveDataBase::acquire() const
{
    PR_Lock(lock_);
}

void
detail::ExclusiveDataBase::release() const
{
    MOZ_RELEASE_ASSERT(PR_Unlock(lock_) == PR_SUCCESS);
}

} // namespace js
