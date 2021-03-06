#include "../gen_peer_wire.c"

void towire_pad_orig(u8 **pptr, size_t num);
#define towire_pad towire_pad_orig
void fromwire_pad_orig(const u8 **cursor, size_t *max, size_t num);
#define towire_pad towire_pad_orig
#define fromwire_pad fromwire_pad_orig
#include "../towire.c"
#include "../fromwire.c"
#undef towire_pad
#undef fromwire_pad

#include <ccan/structeq/structeq.h>
#include <assert.h>
#include <stdio.h>

secp256k1_context *secp256k1_ctx;

/* We allow non-zero padding for testing. */
static const void *towire_pad_arr;
void towire_pad(u8 **pptr, size_t num)
{
	towire_u8_array(pptr, towire_pad_arr, num);
}

static void *fromwire_pad_arr;
void fromwire_pad(const u8 **cursor, size_t *max, size_t num)
{
	fromwire(cursor, max, fromwire_pad_arr, num);
}

/* memsetting pubkeys doesn't work */
static void set_pubkey(struct pubkey *key)
{
	u8 der[PUBKEY_DER_LEN];
	memset(der, 2, sizeof(der));
	assert(pubkey_from_der(der, sizeof(der), key));
}

/* Size up to field. */
#define upto_field(p, field)				\
	((char *)&(p)->field - (char *)(p))

/* Size including field. */
#define with_field(p, field)				\
	(upto_field((p), field) + sizeof((p)->field))

/* Equal upto this field */
#define eq_upto(p1, p2, field)			\
	(memcmp((p1), (p2), upto_field(p1, field)) == 0)

/* Equal upto and including this field */
#define eq_with(p1, p2, field)			\
	(memcmp((p1), (p2), with_field(p1, field)) == 0)

/* Equal from fields first to last inclusive. */
#define eq_between(p1, p2, first, last)					\
	(memcmp((char *)(p1) + upto_field((p1), first),			\
		(char *)(p2) + upto_field((p1), first),			\
		with_field(p1, last) - upto_field(p1, first)) == 0)

/* Equal in one field. */
#define eq_field(p1, p2, field)						\
	(memcmp((char *)(p1) + upto_field((p1), field),			\
		(char *)(p2) + upto_field((p1), field),			\
		sizeof((p1)->field)) == 0)

#define eq_var(p1, p2, field)			\
	(tal_count((p1)->field) == tal_count((p2)->field) && memcmp((p1)->field, (p2)->field, tal_count((p1)->field) * sizeof(*(p1)->field)) == 0)

static inline bool eq_skip_(const void *p1, const void *p2,
			    size_t off, size_t skip, size_t total)
{
	if (memcmp(p1, p2, off) != 0)
		return false;
	p1 = (char *)p1 + off + skip;
	p2 = (char *)p2 + off + skip;
	return memcmp(p1, p2, total - (off + skip)) == 0;
}

/* Convenience structs for everyone! */
struct msg_error {
	struct channel_id channel_id;
	u8 *data;
};
struct msg_closing_signed {
	struct channel_id channel_id;
	u64 fee_satoshis;
	secp256k1_ecdsa_signature signature;
};
struct msg_funding_created {
	struct channel_id temporary_channel_id;
	struct sha256 txid;
	u8 output_index;
	secp256k1_ecdsa_signature signature;
};
struct msg_accept_channel {
	struct channel_id temporary_channel_id;
	u64 dust_limit_satoshis;
	u64 max_htlc_value_in_flight_msat;
	u64 channel_reserve_satoshis;
	u32 minimum_depth;
	u32 htlc_minimum_msat;
	u16 to_self_delay;
	u16 max_accepted_htlcs;
	struct pubkey funding_pubkey;
	struct pubkey revocation_basepoint;
	struct pubkey payment_basepoint;
	struct pubkey delayed_payment_basepoint;
	struct pubkey first_per_commitment_point;
};
struct msg_update_fulfill_htlc {
	struct channel_id channel_id;
	u64 id;
	struct sha256 payment_preimage;
};
struct msg_shutdown {
	struct channel_id channel_id;
	u8 *scriptpubkey;
};
struct msg_funding_signed {
	struct channel_id temporary_channel_id;
	secp256k1_ecdsa_signature signature;
};
struct msg_revoke_and_ack {
	struct channel_id channel_id;
	struct sha256 per_commitment_secret;
	struct pubkey next_per_commitment_point;
	u8 padding[1];
	secp256k1_ecdsa_signature *htlc_timeout_signature;
};
struct msg_channel_update {
	secp256k1_ecdsa_signature signature;
	struct channel_id channel_id;
	u32 timestamp;
	u16 flags;
	u16 expiry;
	u32 htlc_minimum_msat;
	u32 fee_base_msat;
	u32 fee_proportional_millionths;
};
struct msg_funding_locked {
	struct channel_id temporary_channel_id;
	struct channel_id channel_id;
	secp256k1_ecdsa_signature announcement_node_signature;
	secp256k1_ecdsa_signature announcement_bitcoin_signature;
	struct pubkey next_per_commitment_point;
};
struct msg_commit_sig {
	struct channel_id channel_id;
	secp256k1_ecdsa_signature signature;
	secp256k1_ecdsa_signature *htlc_signature;
};
struct msg_node_announcement {
	secp256k1_ecdsa_signature signature;
	u32 timestamp;
	struct pubkey node_id;
	u8 rgb_color[3];
	u8 alias[32];
	u8 *features;
	u8 *addresses;
};
struct msg_open_channel {
	struct channel_id temporary_channel_id;
	u64 funding_satoshis;
	u64 push_msat;
	u64 dust_limit_satoshis;
	u64 max_htlc_value_in_flight_msat;
	u64 channel_reserve_satoshis;
	u32 htlc_minimum_msat;
	u32 feerate_per_kw;
	u16 to_self_delay;
	u16 max_accepted_htlcs;
	struct pubkey funding_pubkey;
	struct pubkey revocation_basepoint;
	struct pubkey payment_basepoint;
	struct pubkey delayed_payment_basepoint;
	struct pubkey first_per_commitment_point;
};
struct msg_update_fail_htlc {
	struct channel_id channel_id;
	u64 id;
	u8 *reason;
};
struct msg_channel_announcement {
	secp256k1_ecdsa_signature node_signature_1;
	secp256k1_ecdsa_signature node_signature_2;
	struct channel_id channel_id;
	secp256k1_ecdsa_signature bitcoin_signature_1;
	secp256k1_ecdsa_signature bitcoin_signature_2;
	struct pubkey node_id_1;
	struct pubkey node_id_2;
	struct pubkey bitcoin_key_1;
	struct pubkey bitcoin_key_2;
	u8 *features;
};
struct msg_init {
	u8 *globalfeatures;
	u8 *localfeatures;
};
struct msg_update_add_htlc {
	struct channel_id channel_id;
	u64 id;
	u32 amount_msat;
	u32 expiry;
	struct sha256 payment_hash;
	u8 onion_routing_packet[1254];
};
struct msg_update_fee {
	struct channel_id channel_id;
	u32 feerate_per_kw;
};

static void *towire_struct_channel_announcement(const tal_t *ctx,
						const struct msg_channel_announcement *s)
{
	return towire_channel_announcement(ctx, 
					   &s->node_signature_1,
					   &s->node_signature_2,
					   &s->channel_id,
					   &s->bitcoin_signature_1,
					   &s->bitcoin_signature_2,
					   &s->node_id_1,
					   &s->node_id_2,
					   &s->bitcoin_key_1,
					   &s->bitcoin_key_2,
					   s->features);
}

static struct msg_channel_announcement *fromwire_struct_channel_announcement(const tal_t *ctx, const void *p, size_t *plen)
{
	struct msg_channel_announcement *s = tal(ctx, struct msg_channel_announcement);
	if (!fromwire_channel_announcement(s, p, plen, 
					  &s->node_signature_1,
					  &s->node_signature_2,
					  &s->channel_id,
					  &s->bitcoin_signature_1,
					  &s->bitcoin_signature_2,
					  &s->node_id_1,
					  &s->node_id_2,
					  &s->bitcoin_key_1,
					  &s->bitcoin_key_2,
					  &s->features))
		return tal_free(s);
	return s;
}

static void *towire_struct_open_channel(const tal_t *ctx,
						const struct msg_open_channel *s)
{
	return towire_open_channel(ctx, 
				   &s->temporary_channel_id,
				   s->funding_satoshis,
				   s->push_msat,
				   s->dust_limit_satoshis,
				   s->max_htlc_value_in_flight_msat,
				   s->channel_reserve_satoshis,
				   s->htlc_minimum_msat,
				   s->feerate_per_kw,
				   s->to_self_delay,
				   s->max_accepted_htlcs,
				   &s->funding_pubkey,
				   &s->revocation_basepoint,
				   &s->payment_basepoint,
				   &s->delayed_payment_basepoint,
				   &s->first_per_commitment_point);
}

static struct msg_open_channel *fromwire_struct_open_channel(const tal_t *ctx, const void *p, size_t *plen)
{
	struct msg_open_channel *s = tal(ctx, struct msg_open_channel);

	if (fromwire_open_channel(p, plen, 
				  &s->temporary_channel_id,
				  &s->funding_satoshis,
				  &s->push_msat,
				  &s->dust_limit_satoshis,
				  &s->max_htlc_value_in_flight_msat,
				  &s->channel_reserve_satoshis,
				  &s->htlc_minimum_msat,
				  &s->feerate_per_kw,
				  &s->to_self_delay,
				  &s->max_accepted_htlcs,
				  &s->funding_pubkey,
				  &s->revocation_basepoint,
				  &s->payment_basepoint,
				  &s->delayed_payment_basepoint,
				  &s->first_per_commitment_point))
		return s;
	return tal_free(s);
}

static void *towire_struct_accept_channel(const tal_t *ctx,
						const struct msg_accept_channel *s)
{
	return towire_accept_channel(ctx, 
				     &s->temporary_channel_id,
				     s->dust_limit_satoshis,
				     s->max_htlc_value_in_flight_msat,
				     s->channel_reserve_satoshis,
				     s->minimum_depth,
				     s->htlc_minimum_msat,
				     s->to_self_delay,
				     s->max_accepted_htlcs,
				     &s->funding_pubkey,
				     &s->revocation_basepoint,
				     &s->payment_basepoint,
				     &s->delayed_payment_basepoint,
				     &s->first_per_commitment_point);
}

static struct msg_accept_channel *fromwire_struct_accept_channel(const tal_t *ctx, const void *p, size_t *plen)
{
	struct msg_accept_channel *s = tal(ctx, struct msg_accept_channel);

	if (fromwire_accept_channel(p, plen, 
				    &s->temporary_channel_id,
				    &s->dust_limit_satoshis,
				    &s->max_htlc_value_in_flight_msat,
				    &s->channel_reserve_satoshis,
				    &s->minimum_depth,
				    &s->htlc_minimum_msat,
				    &s->to_self_delay,
				    &s->max_accepted_htlcs,
				    &s->funding_pubkey,
				    &s->revocation_basepoint,
				    &s->payment_basepoint,
				    &s->delayed_payment_basepoint,
				    &s->first_per_commitment_point))
		return s;
	return tal_free(s);
}

static void *towire_struct_node_announcement(const tal_t *ctx,
				      const struct msg_node_announcement *s)
{
	return towire_node_announcement(ctx, 
					&s->signature,
					s->timestamp,
					&s->node_id,
					s->rgb_color,
					s->alias,
					s->features,
					s->addresses);
}

static struct msg_node_announcement *fromwire_struct_node_announcement(const tal_t *ctx, const void *p, size_t *plen)
{
	struct msg_node_announcement *s = tal(ctx, struct msg_node_announcement);
	if (!fromwire_node_announcement(s, p, plen, 
				       &s->signature,
				       &s->timestamp,
				       &s->node_id,
				       s->rgb_color,
				       s->alias,
					&s->features, &s->addresses))
		return tal_free(s);
	return s;
}

static void *towire_struct_channel_update(const tal_t *ctx,
				      const struct msg_channel_update *s)
{
	return towire_channel_update(ctx, 
				     &s->signature,
				     &s->channel_id,
				     s->timestamp,
				     s->flags,
				     s->expiry,
				     s->htlc_minimum_msat,
				     s->fee_base_msat,
				     s->fee_proportional_millionths);
}

static struct msg_channel_update *fromwire_struct_channel_update(const tal_t *ctx, const void *p, size_t *plen)
{
	struct msg_channel_update *s = tal(ctx, struct msg_channel_update);

	if (fromwire_channel_update(p, plen, 
				    &s->signature,
				    &s->channel_id,
				    &s->timestamp,
				    &s->flags,
				    &s->expiry,
				    &s->htlc_minimum_msat,
				    &s->fee_base_msat,
				    &s->fee_proportional_millionths))
		return s;
	return tal_free(s);
}

static void *towire_struct_funding_locked(const tal_t *ctx,
						const struct msg_funding_locked *s)
{
	return towire_funding_locked(ctx, 
				     &s->temporary_channel_id,
				     &s->channel_id,
				     &s->announcement_node_signature,
				     &s->announcement_bitcoin_signature,
				     &s->next_per_commitment_point);
}

static struct msg_funding_locked *fromwire_struct_funding_locked(const tal_t *ctx, const void *p, size_t *plen)
{
	struct msg_funding_locked *s = tal(ctx, struct msg_funding_locked);

	if (fromwire_funding_locked(p, plen, 
				    &s->temporary_channel_id,
				    &s->channel_id,
				    &s->announcement_node_signature,
				    &s->announcement_bitcoin_signature,
				    &s->next_per_commitment_point))
		return s;
	return tal_free(s);
}

static void *towire_struct_update_fail_htlc(const tal_t *ctx,
					    const struct msg_update_fail_htlc *s)
{
	return towire_update_fail_htlc(ctx, 
				       &s->channel_id,
				       s->id,
				       s->reason);
}

static struct msg_update_fail_htlc *fromwire_struct_update_fail_htlc(const tal_t *ctx, const void *p, size_t *plen)
{
	struct msg_update_fail_htlc *s = tal(ctx, struct msg_update_fail_htlc);

	if (!fromwire_update_fail_htlc(ctx, p, plen, 
				      &s->channel_id,
				      &s->id,
				      &s->reason))
		return tal_free(s);
	return s;

}

static void *towire_struct_update_fulfill_htlc(const tal_t *ctx,
					    const struct msg_update_fulfill_htlc *s)
{
	return towire_update_fulfill_htlc(ctx, 
				       &s->channel_id,
				       s->id,
				       &s->payment_preimage);
}

static struct msg_update_fulfill_htlc *fromwire_struct_update_fulfill_htlc(const tal_t *ctx, const void *p, size_t *plen)
{
	struct msg_update_fulfill_htlc *s = tal(ctx, struct msg_update_fulfill_htlc);

	if (fromwire_update_fulfill_htlc(p, plen, 
				      &s->channel_id,
				      &s->id,
				      &s->payment_preimage))
		return s;
	return tal_free(s);
}

static void *towire_struct_commit_sig(const tal_t *ctx,
				      const struct msg_commit_sig *s)
{
	return towire_commit_sig(ctx, 
				 &s->channel_id,
				 &s->signature,
				 s->htlc_signature);
}

static struct msg_commit_sig *fromwire_struct_commit_sig(const tal_t *ctx, const void *p, size_t *plen)
{
	struct msg_commit_sig *s = tal(ctx, struct msg_commit_sig);

	if (!fromwire_commit_sig(s, p, plen, 
				&s->channel_id,
				&s->signature,
				&s->htlc_signature))
		return tal_free(s);
	return s;
}

static void *towire_struct_revoke_and_ack(const tal_t *ctx,
				      const struct msg_revoke_and_ack *s)
{
	towire_pad_arr = s->padding;
	return towire_revoke_and_ack(ctx, 
				     &s->channel_id,
				     &s->per_commitment_secret,
				     &s->next_per_commitment_point,
				     s->htlc_timeout_signature);
}

static struct msg_revoke_and_ack *fromwire_struct_revoke_and_ack(const tal_t *ctx, const void *p, size_t *plen)
{
	struct msg_revoke_and_ack *s = tal(ctx, struct msg_revoke_and_ack);

	fromwire_pad_arr = s->padding;
	if (!fromwire_revoke_and_ack(s, p, plen, 
				    &s->channel_id,
				    &s->per_commitment_secret,
				    &s->next_per_commitment_point,
				    &s->htlc_timeout_signature))
		return tal_free(s);
	return s;
	
}

static void *towire_struct_funding_signed(const tal_t *ctx,
					  const struct msg_funding_signed *s)
{
	return towire_funding_signed(ctx, 
				     &s->temporary_channel_id,
				     &s->signature);
}

static struct msg_funding_signed *fromwire_struct_funding_signed(const tal_t *ctx, const void *p, size_t *plen)
{
	struct msg_funding_signed *s = tal(ctx, struct msg_funding_signed);

	if (fromwire_funding_signed(p, plen, 
				    &s->temporary_channel_id,
				    &s->signature))
		return s;
	return tal_free(s);
}

static void *towire_struct_closing_signed(const tal_t *ctx,
					  const struct msg_closing_signed *s)
{
	return towire_closing_signed(ctx, 
				     &s->channel_id,
				     s->fee_satoshis,
				     &s->signature);
}

static struct msg_closing_signed *fromwire_struct_closing_signed(const tal_t *ctx, const void *p, size_t *plen)
{
	struct msg_closing_signed *s = tal(ctx, struct msg_closing_signed);

	if (fromwire_closing_signed(p, plen, 
				    &s->channel_id,
				    &s->fee_satoshis,
				    &s->signature))
		return s;
	return tal_free(s);
}

static void *towire_struct_shutdown(const tal_t *ctx,
					  const struct msg_shutdown *s)
{
	return towire_shutdown(ctx, 
			       &s->channel_id,
			       s->scriptpubkey);
}

static struct msg_shutdown *fromwire_struct_shutdown(const tal_t *ctx, const void *p, size_t *plen)
{
	struct msg_shutdown *s = tal(ctx, struct msg_shutdown);

	if (!fromwire_shutdown(s, p, plen, 
			      &s->channel_id,
			      &s->scriptpubkey))
		return tal_free(s);
	return s;
}

static void *towire_struct_funding_created(const tal_t *ctx,
					  const struct msg_funding_created *s)
{
	return towire_funding_created(ctx, 
				      &s->temporary_channel_id,
				      &s->txid,
				      s->output_index,
				      &s->signature);
}

static struct msg_funding_created *fromwire_struct_funding_created(const tal_t *ctx, const void *p, size_t *plen)
{
	struct msg_funding_created *s = tal(ctx, struct msg_funding_created);

	if (fromwire_funding_created(p, plen, 
				     &s->temporary_channel_id,
				     &s->txid,
				     &s->output_index,
				     &s->signature))
		return s;
	return tal_free(s);
}

static void *towire_struct_error(const tal_t *ctx,
				 const struct msg_error *s)
{
	return towire_error(ctx, 
			    &s->channel_id,
			    s->data);
}

static struct msg_error *fromwire_struct_error(const tal_t *ctx, const void *p, size_t *plen)
{
	struct msg_error *s = tal(ctx, struct msg_error);

	if (!fromwire_error(s, p, plen, 
			   &s->channel_id,
			    &s->data))
		return tal_free(s);
	return s;
}

static void *towire_struct_update_add_htlc(const tal_t *ctx,
					  const struct msg_update_add_htlc *s)
{
	return towire_update_add_htlc(ctx, 
				      &s->channel_id,
				      s->id,
				      s->amount_msat,
				      s->expiry,
				      &s->payment_hash,
				      s->onion_routing_packet);
}

static struct msg_update_add_htlc *fromwire_struct_update_add_htlc(const tal_t *ctx, const void *p, size_t *plen)
{
	struct msg_update_add_htlc *s = tal(ctx, struct msg_update_add_htlc);

	if (fromwire_update_add_htlc(p, plen, 
				     &s->channel_id,
				     &s->id,
				     &s->amount_msat,
				     &s->expiry,
				     &s->payment_hash,
				     s->onion_routing_packet))
		return s;
	return tal_free(s);
}


static void *towire_struct_update_fee(const tal_t *ctx,
				      const struct msg_update_fee *s)
{
	return towire_update_fee(ctx, 
				 &s->channel_id,
				 s->feerate_per_kw);
}

static struct msg_update_fee *fromwire_struct_update_fee(const tal_t *ctx, const void *p, size_t *plen)
{
	struct msg_update_fee *s = tal(ctx, struct msg_update_fee);

	if (fromwire_update_fee(p, plen, 
				&s->channel_id,
				&s->feerate_per_kw))
		return s;
	return tal_free(s);
}

static void *towire_struct_init(const tal_t *ctx,
					  const struct msg_init *s)
{
	return towire_init(ctx,
			   s->globalfeatures,
			   s->localfeatures);
}

static struct msg_init *fromwire_struct_init(const tal_t *ctx, const void *p, size_t *plen)
{
	struct msg_init *s = tal(ctx, struct msg_init);

	if (!fromwire_init(s, p, plen, 
			  &s->globalfeatures,
			  &s->localfeatures))
		return tal_free(s);

	return s;
}

static bool channel_announcement_eq(const struct msg_channel_announcement *a,
				    const struct msg_channel_announcement *b)
{
	return eq_upto(a, b, features)
		&& eq_var(a, b, features);
}

static bool funding_locked_eq(const struct msg_funding_locked *a,
			      const struct msg_funding_locked *b)
{
	return structeq(a, b);
}

static bool update_fail_htlc_eq(const struct msg_update_fail_htlc *a,
				const struct msg_update_fail_htlc *b)
{
	return eq_with(a, b, id)
		&& eq_var(a, b, reason);
}

static bool commit_sig_eq(const struct msg_commit_sig *a,
			  const struct msg_commit_sig *b)
{
	return eq_upto(a, b, htlc_signature)
		&& eq_var(a, b, htlc_signature);
}

static bool funding_signed_eq(const struct msg_funding_signed *a,
			      const struct msg_funding_signed *b)
{
	return structeq(a, b);
}

static bool closing_signed_eq(const struct msg_closing_signed *a,
			      const struct msg_closing_signed *b)
{
	return structeq(a, b);
}

static bool update_fulfill_htlc_eq(const struct msg_update_fulfill_htlc *a,
				   const struct msg_update_fulfill_htlc *b)
{
	return structeq(a, b);
}

static bool error_eq(const struct msg_error *a,
		     const struct msg_error *b)
{
	return eq_upto(a, b, data)
		&& eq_var(a, b, data);
}

static bool init_eq(const struct msg_init *a,
		    const struct msg_init *b)
{
	return eq_var(a, b, globalfeatures)
		&& eq_var(a, b, localfeatures);
}

static bool update_fee_eq(const struct msg_update_fee *a,
			  const struct msg_update_fee *b)
{
	return structeq(a, b);
}

static bool shutdown_eq(const struct msg_shutdown *a,
			const struct msg_shutdown *b)
{
	return eq_upto(a, b, scriptpubkey)
		&& eq_var(a, b, scriptpubkey);
}

static bool funding_created_eq(const struct msg_funding_created *a,
			       const struct msg_funding_created *b)
{
	return eq_with(a, b, output_index)
		&& eq_field(a, b, signature);
}

static bool revoke_and_ack_eq(const struct msg_revoke_and_ack *a,
			      const struct msg_revoke_and_ack *b)
{
	return eq_with(a, b, padding)
		&& eq_var(a, b, htlc_timeout_signature);
}

static bool open_channel_eq(const struct msg_open_channel *a,
			    const struct msg_open_channel *b)
{
	return eq_with(a, b, max_accepted_htlcs)
		&& eq_between(a, b, funding_pubkey, first_per_commitment_point);
}

static bool channel_update_eq(const struct msg_channel_update *a,
			      const struct msg_channel_update *b)
{
	return structeq(a, b);
}

static bool accept_channel_eq(const struct msg_accept_channel *a,
			      const struct msg_accept_channel *b)
{
	return eq_with(a, b, max_accepted_htlcs)
		&& eq_between(a, b, funding_pubkey, first_per_commitment_point);
}

static bool update_add_htlc_eq(const struct msg_update_add_htlc *a,
			       const struct msg_update_add_htlc *b)
{
	return eq_with(a, b, onion_routing_packet);
}

static bool node_announcement_eq(const struct msg_node_announcement *a,
				 const struct msg_node_announcement *b)
{
	return eq_with(a, b, alias)
		&& eq_var(a, b, features)
		&& eq_var(a, b, addresses);
}

/* Try flipping each bit, try running short. */
#define test_corruption(a, b, type)				\
	for (i = 0; i < tal_count(msg) * 8; i++) {		\
		len = tal_count(msg);				\
		msg[i / 8] ^= (1 << (i%8));			\
		b = fromwire_struct_##type(ctx, msg, &len);	\
		assert(!b || !type##_eq(a, b));			\
		msg[i / 8] ^= (1 << (i%8));			\
	}							\
	for (i = 0; i < tal_count(msg); i++) {			\
		len = i;					\
		b = fromwire_struct_##type(ctx, msg, &len);	\
		assert(!b);					\
	}

int main(void)
{
	struct msg_channel_announcement ca, *ca2;
	struct msg_funding_locked fl, *fl2;
	struct msg_update_fail_htlc ufh, *ufh2;
	struct msg_commit_sig cs, *cs2;
	struct msg_funding_signed fs, *fs2;
	struct msg_closing_signed cls, *cls2;
	struct msg_update_fulfill_htlc uflh, *uflh2;
	struct msg_error e, *e2;
	struct msg_init init, *init2;
	struct msg_update_fee uf, *uf2;
	struct msg_shutdown shutdown, *shutdown2;
	struct msg_funding_created fc, *fc2;
	struct msg_revoke_and_ack raa, *raa2;
	struct msg_open_channel oc, *oc2;
	struct msg_channel_update cu, *cu2;
	struct msg_accept_channel ac, *ac2;
	struct msg_update_add_htlc uah, *uah2;
	struct msg_node_announcement na, *na2;
	void *ctx = tal(NULL, char);
	size_t i, len;
	u8 *msg;

	secp256k1_ctx = secp256k1_context_create(SECP256K1_CONTEXT_VERIFY
						 | SECP256K1_CONTEXT_SIGN);

	memset(&ca, 2, sizeof(ca));
	set_pubkey(&ca.node_id_1);
	set_pubkey(&ca.node_id_2);
	set_pubkey(&ca.bitcoin_key_1);
	set_pubkey(&ca.bitcoin_key_2);
 	ca.features = tal_arr(ctx, u8, 2);
 	memset(ca.features, 2, 2);
	
	msg = towire_struct_channel_announcement(ctx, &ca);
	len = tal_count(msg);
	ca2 = fromwire_struct_channel_announcement(ctx, msg, &len);
	assert(len == 0);
	assert(channel_announcement_eq(&ca, ca2));
	test_corruption(&ca, ca2, channel_announcement);

	memset(&fl, 2, sizeof(fl));
	set_pubkey(&fl.next_per_commitment_point);
	
	msg = towire_struct_funding_locked(ctx, &fl);
	len = tal_count(msg);
	fl2 = fromwire_struct_funding_locked(ctx, msg, &len);
	assert(len == 0);
	assert(funding_locked_eq(&fl, fl2));
	test_corruption(&fl, fl2, funding_locked);
	
	memset(&ufh, 2, sizeof(ufh));
 	ufh.reason = tal_arr(ctx, u8, 2);
 	memset(ufh.reason, 2, 2);
	
	msg = towire_struct_update_fail_htlc(ctx, &ufh);
	len = tal_count(msg);
	ufh2 = fromwire_struct_update_fail_htlc(ctx, msg, &len);
	assert(len == 0);
	assert(update_fail_htlc_eq(&ufh, ufh2));
	test_corruption(&ufh, ufh2, update_fail_htlc);

	memset(&cs, 2, sizeof(cs));
	cs.htlc_signature = tal_arr(ctx, secp256k1_ecdsa_signature, 2);
	memset(cs.htlc_signature, 2, sizeof(secp256k1_ecdsa_signature)*2);
	
	msg = towire_struct_commit_sig(ctx, &cs);
	len = tal_count(msg);
	cs2 = fromwire_struct_commit_sig(ctx, msg, &len);
	assert(len == 0);
	assert(commit_sig_eq(&cs, cs2));
	test_corruption(&cs, cs2, commit_sig);

	memset(&fs, 2, sizeof(fs));
	
	msg = towire_struct_funding_signed(ctx, &fs);
	len = tal_count(msg);
	fs2 = fromwire_struct_funding_signed(ctx, msg, &len);
	assert(len == 0);
	assert(funding_signed_eq(&fs, fs2));
	test_corruption(&fs, fs2, funding_signed);

	memset(&cls, 2, sizeof(cls));
	
	msg = towire_struct_closing_signed(ctx, &cls);
	len = tal_count(msg);
	cls2 = fromwire_struct_closing_signed(ctx, msg, &len);
	assert(len == 0);
	assert(closing_signed_eq(&cls, cls2));
	test_corruption(&cls, cls2, closing_signed);
	
	memset(&uflh, 2, sizeof(uflh));
	
	msg = towire_struct_update_fulfill_htlc(ctx, &uflh);
	len = tal_count(msg);
	uflh2 = fromwire_struct_update_fulfill_htlc(ctx, msg, &len);
	assert(len == 0);
	assert(update_fulfill_htlc_eq(&uflh, uflh2));
	test_corruption(&uflh, uflh2, update_fulfill_htlc);

	memset(&e, 2, sizeof(e));
	e.data = tal_arr(ctx, u8, 2);
	memset(e.data, 2, 2);
	
	msg = towire_struct_error(ctx, &e);
	len = tal_count(msg);
	e2 = fromwire_struct_error(ctx, msg, &len);
	assert(len == 0);
	assert(error_eq(&e, e2));
	test_corruption(&e, e2, error);

	memset(&init, 2, sizeof(init));
	init.globalfeatures = tal_arr(ctx, u8, 2);
	memset(init.globalfeatures, 2, 2);
	init.localfeatures = tal_arr(ctx, u8, 2);
	memset(init.localfeatures, 2, 2);

	msg = towire_struct_init(ctx, &init);
	len = tal_count(msg);
	init2 = fromwire_struct_init(ctx, msg, &len);
	assert(len == 0);
	assert(init_eq(&init, init2));
	test_corruption(&init, init2, init);

	memset(&uf, 2, sizeof(uf));
	
	msg = towire_struct_update_fee(ctx, &uf);
	len = tal_count(msg);
	uf2 = fromwire_struct_update_fee(ctx, msg, &len);
	assert(len == 0);
	assert(update_fee_eq(&uf, uf2));
	test_corruption(&uf, uf2, update_fee);

	memset(&shutdown, 2, sizeof(shutdown));
	shutdown.scriptpubkey = tal_arr(ctx, u8, 2);
	memset(shutdown.scriptpubkey, 2, 2);
	
	msg = towire_struct_shutdown(ctx, &shutdown);
	len = tal_count(msg);
	shutdown2 = fromwire_struct_shutdown(ctx, msg, &len);
	assert(len == 0);
	assert(shutdown_eq(&shutdown, shutdown2));
	test_corruption(&shutdown, shutdown2, shutdown);
	
	memset(&fc, 2, sizeof(fc));
	
	msg = towire_struct_funding_created(ctx, &fc);
	len = tal_count(msg);
	fc2 = fromwire_struct_funding_created(ctx, msg, &len);
	assert(len == 0);
	assert(funding_created_eq(&fc, fc2));
	test_corruption(&fc, fc2, funding_created);

	memset(&raa, 2, sizeof(raa));
	set_pubkey(&raa.next_per_commitment_point);
	raa.htlc_timeout_signature = tal_arr(ctx, secp256k1_ecdsa_signature, 2);
	memset(raa.htlc_timeout_signature, 2, sizeof(secp256k1_ecdsa_signature) * 2);
	
	msg = towire_struct_revoke_and_ack(ctx, &raa);
	len = tal_count(msg);
	raa2 = fromwire_struct_revoke_and_ack(ctx, msg, &len);
	assert(len == 0);
	assert(revoke_and_ack_eq(&raa, raa2));
	test_corruption(&raa, raa2, revoke_and_ack);

	memset(&oc, 2, sizeof(oc));
	set_pubkey(&oc.funding_pubkey);
	set_pubkey(&oc.revocation_basepoint);
	set_pubkey(&oc.payment_basepoint);
	set_pubkey(&oc.delayed_payment_basepoint);
	set_pubkey(&oc.first_per_commitment_point);
	
	msg = towire_struct_open_channel(ctx, &oc);
	len = tal_count(msg);
	oc2 = fromwire_struct_open_channel(ctx, msg, &len);
	assert(len == 0);
	assert(open_channel_eq(&oc, oc2));
	test_corruption(&oc, oc2, open_channel);

	memset(&cu, 2, sizeof(cu));
	
	msg = towire_struct_channel_update(ctx, &cu);
	len = tal_count(msg);
	cu2 = fromwire_struct_channel_update(ctx, msg, &len);
	assert(len == 0);
	assert(channel_update_eq(&cu, cu2));
	test_corruption(&cu, cu2, channel_update);

	memset(&ac, 2, sizeof(ac));
	set_pubkey(&ac.funding_pubkey);
	set_pubkey(&ac.revocation_basepoint);
	set_pubkey(&ac.payment_basepoint);
	set_pubkey(&ac.delayed_payment_basepoint);
	set_pubkey(&ac.first_per_commitment_point);
	
	msg = towire_struct_accept_channel(ctx, &ac);
	len = tal_count(msg);
	ac2 = fromwire_struct_accept_channel(ctx, msg, &len);
	assert(len == 0);
	assert(accept_channel_eq(&ac, ac2));
	test_corruption(&ac, ac2, accept_channel);

	memset(&uah, 2, sizeof(uah));
	
	msg = towire_struct_update_add_htlc(ctx, &uah);
	len = tal_count(msg);
	uah2 = fromwire_struct_update_add_htlc(ctx, msg, &len);
	assert(len == 0);
	assert(update_add_htlc_eq(&uah, uah2));
	test_corruption(&uah, uah2, update_add_htlc);

	memset(&na, 2, sizeof(na));
	set_pubkey(&na.node_id);
	na.features = tal_arr(ctx, u8, 2);
	memset(na.features, 2, 2);
	na.addresses = tal_arr(ctx, u8, 2);
	memset(na.addresses, 2, 2);

	msg = towire_struct_node_announcement(ctx, &na);
	len = tal_count(msg);
	na2 = fromwire_struct_node_announcement(ctx, msg, &len);
	assert(len == 0);
	assert(node_announcement_eq(&na, na2));
	test_corruption(&na, na2, node_announcement);

	/* No memory leaks please */
	secp256k1_context_destroy(secp256k1_ctx);
	tal_free(ctx);
	return 0;
}
