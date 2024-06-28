#include <mizugaki/analyzer/details/analyzer_context.h>

#include <takatori/util/exception.h>

namespace mizugaki::analyzer::details {

using ::takatori::util::finalizer;
using ::takatori::util::optional_ptr;
using ::takatori::util::sequence_view;
using ::takatori::util::throw_exception;

finalizer analyzer_context::initialize(
        options_type const& options,
        optional_ptr<::takatori::document::document const> source,
        sequence_view<ast::node_region const> comments,
        placeholder_map const& placeholders,
        optional_ptr<::yugawara::variable::provider const> host_parameters) {
    bool b = false;
    if (!initialized_.compare_exchange_strong(
            b, true,
            std::memory_order_acquire, std::memory_order_relaxed)) {
        throw_exception(std::runtime_error("analyzer context is in use"));
    }

    finalizer f {
        [this] { finalize(); }
    };

    options_ = options;
    source_ = source;
    comments_ = comments;
    placeholders_ = placeholders;
    host_parameters_ = host_parameters;

    diagnostics_.clear();
    types_.clear();
    values_.clear();

    return f;
}

void analyzer_context::finalize() {
    options_ = {};
    source_ = {};
    comments_ = {};
    placeholders_ = {};
    host_parameters_ = {};

    diagnostics_.clear();
    types_.clear();
    values_.clear();

    initialized_.store(false, std::memory_order_release);
}

} // namespace mizugaki::analyzer::details
