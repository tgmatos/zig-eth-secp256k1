#define SECP256K1_BUILD
#include <stdlib.h>
#include <secp256k1_recovery.h>
#include <secp256k1.h>
#include <secp256k1.h>
#include <string.h>
#include <assert.h>
#include <sys/random.h>

// Copyright 2015 Jeffrey Wilcke, Felix Lange, Gustav Simonsson. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be found in
// the LICENSE file.

static int fill_random(unsigned char* data, size_t size) {
  ssize_t res = getrandom(data, size, 0);
  if (res < 0 || (size_t)res != size ) {
    return 0;
  } else {
    return 1;
  }
}

// secp256k1_context_create_sign_verify creates a context for signing and signature verification.
secp256k1_context *secp256k1_context_create_sign_verify()
{
  unsigned char randomize[32];
  secp256k1_context* ctx = secp256k1_context_create(SECP256K1_CONTEXT_SIGN | SECP256K1_CONTEXT_VERIFY);

  if(!fill_random(randomize, sizeof(randomize))) {return NULL;}
  int return_val = secp256k1_context_randomize(ctx, randomize);
  assert(return_val);
  return ctx;
}

// secp256k1_ext_ecdsa_recover recovers the public key of an encoded compact signature.
//
// Returns: 1: recovery was successful
//          0: recovery was not successful
// Args:    ctx:        pointer to a context object (cannot be NULL)
//  Out:    pubkey_out: the serialized 65-byte public key of the signer (cannot be NULL)
//  In:     sigdata:    pointer to a 65-byte signature with the recovery id at the end (cannot be NULL)
//          msgdata:    pointer to a 32-byte message (cannot be NULL)
int secp256k1_ext_ecdsa_recover(
    const secp256k1_context *ctx,
    unsigned char *pubkey_out,
    const unsigned char *sigdata,
    const unsigned char *msgdata)
{
    secp256k1_ecdsa_recoverable_signature sig;
    secp256k1_pubkey pubkey;

    if (!secp256k1_ecdsa_recoverable_signature_parse_compact(ctx, &sig, sigdata, (int)sigdata[64]))
    {
        return 0;
    }
    if (!secp256k1_ecdsa_recover(ctx, &pubkey, &sig, msgdata))
    {
        return 0;
    }
    size_t outputlen = 65;
    return secp256k1_ec_pubkey_serialize(ctx, pubkey_out, &outputlen, &pubkey, SECP256K1_EC_UNCOMPRESSED);
}

// secp256k1_ext_ecdsa_verify verifies an encoded compact signature.
//
// Returns: 1: signature is valid
//          0: signature is invalid
// Args:    ctx:        pointer to a context object (cannot be NULL)
//  In:     sigdata:    pointer to a 64-byte signature (cannot be NULL)
//          msgdata:    pointer to a 32-byte message (cannot be NULL)
//          pubkeydata: pointer to public key data (cannot be NULL)
//          pubkeylen:  length of pubkeydata
int secp256k1_ext_ecdsa_verify(
    const secp256k1_context *ctx,
    const unsigned char *sigdata,
    const unsigned char *msgdata,
    const unsigned char *pubkeydata,
    size_t pubkeylen)
{
    secp256k1_ecdsa_signature sig;
    secp256k1_pubkey pubkey;

    if (!secp256k1_ecdsa_signature_parse_compact(ctx, &sig, sigdata))
    {
        return 0;
    }
    if (!secp256k1_ec_pubkey_parse(ctx, &pubkey, pubkeydata, pubkeylen))
    {
        return 0;
    }
    return secp256k1_ecdsa_verify(ctx, &sig, msgdata, &pubkey);
}

// secp256k1_ext_reencode_pubkey decodes then encodes a public key. It can be used to
// convert between public key formats. The input/output formats are chosen depending on the
// length of the input/output buffers.
//
// Returns: 1: conversion successful
//          0: conversion unsuccessful
// Args:    ctx:        pointer to a context object (cannot be NULL)
//  Out:    out:        output buffer that will contain the reencoded key (cannot be NULL)
//  In:     outlen:     length of out (33 for compressed keys, 65 for uncompressed keys)
//          pubkeydata: the input public key (cannot be NULL)
//          pubkeylen:  length of pubkeydata
int secp256k1_ext_reencode_pubkey(
    const secp256k1_context *ctx,
    unsigned char *out,
    size_t outlen,
    const unsigned char *pubkeydata,
    size_t pubkeylen)
{
    secp256k1_pubkey pubkey;

    if (!secp256k1_ec_pubkey_parse(ctx, &pubkey, pubkeydata, pubkeylen))
    {
        return 0;
    }
    unsigned int flag = (outlen == 33) ? SECP256K1_EC_COMPRESSED : SECP256K1_EC_UNCOMPRESSED;
    return secp256k1_ec_pubkey_serialize(ctx, out, &outlen, &pubkey, flag);
}

// secp256k1_ext_scalar_mul multiplies a point by a scalar in constant time.
//
// Returns: 1: multiplication was successful
//          0: scalar was invalid (zero or overflow)
// Args:    ctx:      pointer to a context object (cannot be NULL)
//  Out:    point:    the multiplied point (usually secret)
//  In:     point:    pointer to a 64-byte public point,
//                    encoded as two 256bit big-endian numbers.
//          scalar:   a 32-byte scalar with which to multiply the point
int secp256k1_ext_scalar_mul(const secp256k1_context *ctx, unsigned char *point, const unsigned char *scalar) {
    int ret = 0;
    secp256k1_pubkey pubkey;

    assert(ctx != NULL);
    assert(point != NULL);
    assert(scalar != NULL);

    // Check if scalar is zero
    int is_zero = 1;
    for (int i = 0; i < 32; i++) {
        if (scalar[i] != 0) {
            is_zero = 0;
            break;
        }
    }
    if (is_zero) {
        return 0;
    }

    if (!secp256k1_ec_pubkey_parse(ctx, &pubkey, point, 64)) {
        return 0;
    }

    if (secp256k1_ec_pubkey_tweak_mul(ctx, &pubkey, scalar)) {
        size_t output_len = 64;
        if (secp256k1_ec_pubkey_serialize(ctx, point, &output_len, &pubkey, SECP256K1_EC_UNCOMPRESSED)) {
            ret = 1;
        }
    }

    return ret;
}
