/*
 * Copyright (C) Advanced Micro Devices, Inc. 2019. ALL RIGHTS RESERVED.
 * See file LICENSE for terms.
 */

#include "rocm_copy_ep.h"
#include "rocm_copy_iface.h"

#include <uct/base/uct_log.h>
#include <ucs/debug/memtrack.h>
#include <ucs/type/class.h>

static UCS_CLASS_INIT_FUNC(uct_rocm_copy_ep_t, const uct_ep_params_t *params)
{
    START_TRACE();
    uct_rocm_copy_iface_t *iface = ucs_derived_of(params->iface, uct_rocm_copy_iface_t);

    UCS_CLASS_CALL_SUPER_INIT(uct_base_ep_t, &iface->super);
    STOP_TRACE();
    return UCS_OK;
}

static UCS_CLASS_CLEANUP_FUNC(uct_rocm_copy_ep_t)
{
}

UCS_CLASS_DEFINE(uct_rocm_copy_ep_t, uct_base_ep_t)
UCS_CLASS_DEFINE_NEW_FUNC(uct_rocm_copy_ep_t, uct_ep_t, const uct_ep_params_t *);
UCS_CLASS_DEFINE_DELETE_FUNC(uct_rocm_copy_ep_t, uct_ep_t);

#define uct_rocm_copy_trace_data(_remote_addr, _rkey, _fmt, ...) \
     ucs_trace_data(_fmt " to %"PRIx64"(%+ld)", ## __VA_ARGS__, (_remote_addr), \
                    (_rkey))

static UCS_F_ALWAYS_INLINE ucs_status_t
uct_rocm_copy_ep_zcopy(uct_ep_h tl_ep,
                                   uint64_t remote_addr,
                                   const uct_iov_t *iov,
                                   int is_put)
{
    START_TRACE();
    size_t size = uct_iov_get_length(iov);

    if (!size) {
        STOP_TRACE();
        return UCS_OK;
    }

    if (is_put)
    {
        memcpy((void *)remote_addr, iov->buffer, size);
        fprintf(stdout, "--- Performing rocm_copy memcpy from %p to (remote) %p\n", iov->buffer, (void *)remote_addr);
        fflush(stdout);

    }
    else
    {
        memcpy(iov->buffer, (void *)remote_addr, size);
        fprintf(stdout, "--- Performing rocm_copy memcpy from (remote) %p to %p\n", (void *)remote_addr, iov->buffer);
        fflush(stdout);
    }
    STOP_TRACE();
    return UCS_OK;
}

ucs_status_t uct_rocm_copy_ep_get_zcopy(uct_ep_h tl_ep, const uct_iov_t *iov, size_t iovcnt,
                                        uint64_t remote_addr, uct_rkey_t rkey,
                                        uct_completion_t *comp)
{
    START_TRACE();
    ucs_status_t status;

    status = uct_rocm_copy_ep_zcopy(tl_ep, remote_addr, iov, 0);

    UCT_TL_EP_STAT_OP(ucs_derived_of(tl_ep, uct_base_ep_t), GET, ZCOPY,
                      uct_iov_total_length(iov, iovcnt));
    uct_rocm_copy_trace_data(remote_addr, rkey, "GET_ZCOPY [length %zu]",
                             uct_iov_total_length(iov, iovcnt));
    STOP_TRACE();
    return status;
}

ucs_status_t uct_rocm_copy_ep_put_zcopy(uct_ep_h tl_ep, const uct_iov_t *iov, size_t iovcnt,
                                        uint64_t remote_addr, uct_rkey_t rkey,
                                        uct_completion_t *comp)
{
    START_TRACE();
    ucs_status_t status;

    status = uct_rocm_copy_ep_zcopy(tl_ep, remote_addr, iov, 1);

    UCT_TL_EP_STAT_OP(ucs_derived_of(tl_ep, uct_base_ep_t), PUT, ZCOPY,
                      uct_iov_total_length(iov, iovcnt));
    uct_rocm_copy_trace_data(remote_addr, rkey, "GET_ZCOPY [length %zu]",
                             uct_iov_total_length(iov, iovcnt));
    STOP_TRACE();
    return status;

}


ucs_status_t uct_rocm_copy_ep_put_short(uct_ep_h tl_ep, const void *buffer,
                                        unsigned length, uint64_t remote_addr,
                                        uct_rkey_t rkey)
{
    START_TRACE();
    memcpy((void *)remote_addr, buffer, length);
    fprintf(stdout, "--- Performing rocm_copy memcpy (put_short) from %p to (remote) %p\n", buffer, (void *)remote_addr);
    fflush(stdout);


    UCT_TL_EP_STAT_OP(ucs_derived_of(tl_ep, uct_base_ep_t), PUT, SHORT, length);
    ucs_trace_data("PUT_SHORT size %d from %p to %p",
                   length, buffer, (void *)remote_addr);
    STOP_TRACE();
    return UCS_OK;
}

ucs_status_t uct_rocm_copy_ep_get_short(uct_ep_h tl_ep, void *buffer,
                                        unsigned length, uint64_t remote_addr,
                                        uct_rkey_t rkey)
{
    START_TRACE();
    /* device to host */
    memcpy(buffer, (void *)remote_addr, length);
    fprintf(stdout, "--- Performing rocm_copy memcpy (get_short) from %p to (remote) %p\n", (void *)remote_addr, buffer);
    fflush(stdout);

    UCT_TL_EP_STAT_OP(ucs_derived_of(tl_ep, uct_base_ep_t), GET, SHORT, length);
    ucs_trace_data("GET_SHORT size %d from %p to %p",
                   length, (void *)remote_addr, buffer);
    STOP_TRACE();
    return UCS_OK;
}
