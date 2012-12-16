/*
 * Copyright (c) 2010-2011 Anil Madhavapeddy <anil@recoil.org>
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include <stdint.h>
#include <assert.h>

#include <xen/io/netif.h>
#include <xen/io/blkif.h>
#include <xen/io/console.h>
#include <xen/io/xs_wire.h>

#include <caml/mlvalues.h>
#include <caml/alloc.h>
#include <caml/memory.h>
#include <caml/bigarray.h>

#define PAGE_SIZE 4096

#define xen_mb() __sync_synchronize()
#define xen_wmb() __sync_synchronize()


/* Shared ring with request/response structs */

struct sring {
  RING_IDX req_prod, req_event;
  RING_IDX rsp_prod, rsp_event;
  uint8_t  pad[64];
};

#define SRING_VAL(x) ((struct sring *)(Caml_ba_data_val(Field(x,0))))

CAMLprim value
caml_sring_rsp_prod(value v_sring)
{
  return Val_int(SRING_VAL(v_sring)->rsp_prod);
}

CAMLprim value
caml_sring_req_prod(value v_sring)
{
  return Val_int(SRING_VAL(v_sring)->req_prod);
}

CAMLprim value
caml_sring_req_event(value v_sring)
{
  xen_mb ();
  return Val_int(SRING_VAL(v_sring)->req_event);
}

CAMLprim value
caml_sring_rsp_event(value v_sring)
{
  xen_mb ();
  return Val_int(SRING_VAL(v_sring)->rsp_event);
}

CAMLprim value
caml_sring_push_requests(value v_sring, value v_req_prod_pvt)
{
  struct sring *sring = SRING_VAL(v_sring);
  assert(((unsigned long)sring % PAGE_SIZE) == 0);
  xen_wmb(); /* ensure requests are seen before the index is updated */
  sring->req_prod = Int_val(v_req_prod_pvt);
  return Val_unit;
}

CAMLprim value
caml_sring_push_responses(value v_sring, value v_rsp_prod_pvt)
{
  struct sring *sring = SRING_VAL(v_sring);
  xen_wmb(); /* ensure requests are seen before the index is updated */
  sring->rsp_prod = Int_val(v_rsp_prod_pvt);
  return Val_unit;
}

CAMLprim value
caml_sring_set_rsp_event(value v_sring, value v_rsp_cons)
{
  struct sring *sring = SRING_VAL(v_sring);
  sring->rsp_event = Int_val(v_rsp_cons);
  xen_mb();
  return Val_unit;
}

CAMLprim value
caml_sring_set_req_event(value v_sring, value v_req_cons)
{
  struct sring *sring = SRING_VAL(v_sring);
  sring->req_event = Int_val(v_req_cons);
  xen_mb();
  return Val_unit;
}

