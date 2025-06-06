/*
 * Copyright (c) 2024, Aliaksandr Kalenik <kalenik.aliaksandr@gmail.com>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include <LibWeb/Bindings/ExceptionOrUtils.h>
#include <LibWeb/WebIDL/ObservableArray.h>

namespace Web::WebIDL {

GC_DEFINE_ALLOCATOR(ObservableArray);

GC::Ref<ObservableArray> ObservableArray::create(JS::Realm& realm)
{
    auto prototype = realm.intrinsics().array_prototype();
    return realm.create<ObservableArray>(prototype);
}

ObservableArray::ObservableArray(Object& prototype)
    : JS::Array(prototype)
{
}

void ObservableArray::visit_edges(JS::Cell::Visitor& visitor)
{
    Base::visit_edges(visitor);
    visitor.visit(m_on_set_an_indexed_value);
    visitor.visit(m_on_delete_an_indexed_value);
}

void ObservableArray::set_on_set_an_indexed_value_callback(SetAnIndexedValueCallbackFunction&& callback)
{
    m_on_set_an_indexed_value = GC::create_function(heap(), move(callback));
}

void ObservableArray::set_on_delete_an_indexed_value_callback(DeleteAnIndexedValueCallbackFunction&& callback)
{
    m_on_delete_an_indexed_value = GC::create_function(heap(), move(callback));
}

JS::ThrowCompletionOr<bool> ObservableArray::internal_set(JS::PropertyKey const& property_key, JS::Value value, JS::Value receiver, JS::CacheablePropertyMetadata* metadata)
{
    if (property_key.is_number() && m_on_set_an_indexed_value)
        TRY(Bindings::throw_dom_exception_if_needed(vm(), [&] { return m_on_set_an_indexed_value->function()(value); }));
    return TRY(Base::internal_set(property_key, value, receiver, metadata));
}

JS::ThrowCompletionOr<bool> ObservableArray::internal_delete(JS::PropertyKey const& property_key)
{
    if (property_key.is_number() && m_on_delete_an_indexed_value) {
        auto maybe_value_and_attributes = indexed_properties().get(property_key.as_number());
        JS::Value deleted_value;
        if (maybe_value_and_attributes.has_value())
            deleted_value = maybe_value_and_attributes->value;
        TRY(Bindings::throw_dom_exception_if_needed(vm(), [&] { return m_on_delete_an_indexed_value->function()(deleted_value); }));
    }
    return JS::Array::internal_delete(property_key);
}

JS::ThrowCompletionOr<void> ObservableArray::append(JS::Value value)
{
    if (m_on_set_an_indexed_value)
        TRY(Bindings::throw_dom_exception_if_needed(vm(), [&] { return m_on_set_an_indexed_value->function()(value); }));
    indexed_properties().append(value);
    return {};
}

void ObservableArray::clear()
{
    while (!indexed_properties().is_empty()) {
        auto deleted_value = indexed_properties().storage()->take_first().value;
        MUST(m_on_delete_an_indexed_value->function()(deleted_value));
    }
}

}
