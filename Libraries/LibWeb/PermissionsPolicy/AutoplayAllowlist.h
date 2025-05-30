/*
 * Copyright (c) 2023-2025, Tim Flynn <trflynn89@ladybird.org>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#pragma once

#include <AK/Forward.h>
#include <AK/Variant.h>
#include <AK/Vector.h>
#include <LibURL/Forward.h>
#include <LibWeb/Forward.h>
#include <LibWeb/PermissionsPolicy/Decision.h>

namespace Web::PermissionsPolicy {

class AutoplayAllowlist {
public:
    static AutoplayAllowlist& the();

    Decision is_allowed_for_origin(DOM::Document const&, URL::Origin const&) const;

    void enable_globally();
    void enable_for_origins(ReadonlySpan<String>);

private:
    AutoplayAllowlist();
    ~AutoplayAllowlist();

    using Patterns = Vector<URL::Origin>;
    struct Global { };

    Optional<Variant<Patterns, Global>> m_allowlist;
};

}
