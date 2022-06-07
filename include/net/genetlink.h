/* SPDX-License-Identifier: GPL-2.0 */
#ifndef __NET_GENERIC_NETLINK_H
#define __NET_GENERIC_NETLINK_H

#include <linux/genetlink.h>
#include <net/netlink.h>
#include <net/net_namespace.h>
#include <linux/version.h>

#define GENLMSG_DEFAULT_SIZE (NLMSG_DEFAULT_SIZE - GENL_HDRLEN)

/**
 * struct genl_multicast_group - generic netlink multicast group
 * @name: name of the multicast group, names are per-family
 */
struct genl_multicast_group {
	char			name[GENL_NAMSIZ];
};

struct genl_ops;
struct genl_info;

/**
 * struct genl_family - generic netlink family
 * @id: protocol family identifier (private)
 * @hdrsize: length of user specific header in bytes
 * @name: name of family
 * @version: protocol version
 * @maxattr: maximum number of attributes supported
 * @policy: netlink policy
 * @netnsok: set to true if the family can handle network
 *	namespaces and should be presented in all of them
 * @parallel_ops: operations can be called in parallel and aren't
 *	synchronized by the core genetlink code
 * @pre_doit: called before an operation's doit callback, it may
 *	do additional, common, filtering and return an error
 * @post_doit: called after an operation's doit callback, it may
 *	undo operations done by pre_doit, for example release locks
 * @mcgrps: multicast groups used by this family
 * @n_mcgrps: number of multicast groups
 * @mcgrp_offset: starting number of multicast group IDs in this family
 *	(private)
 * @ops: the operations supported by this family
 * @n_ops: number of operations supported by this family
 * @small_ops: the small-struct operations supported by this family
 * @n_small_ops: number of small-struct operations supported by this family
 */
struct genl_family {
	int			id;		/* private */
	unsigned int		hdrsize;
	char			name[GENL_NAMSIZ];
	unsigned int		version;
	unsigned int		maxattr;
	unsigned int		mcgrp_offset;	/* private */
	u8			netnsok:1;
	u8			parallel_ops:1;
	u8			n_ops;
	u8			n_small_ops;
	u8			n_mcgrps;
	const struct nla_policy *policy;
	int			(*pre_doit)(const struct genl_ops *ops,
					    struct sk_buff *skb,
					    struct genl_info *info);
	void			(*post_doit)(const struct genl_ops *ops,
					     struct sk_buff *skb,
					     struct genl_info *info);
	const struct genl_ops *	ops;
	const struct genl_small_ops *small_ops;
	const struct genl_multicast_group *mcgrps;
	struct module		*module;
};

/**
 * struct genl_info - receiving information
 * @snd_seq: sending sequence number
 * @snd_portid: netlink portid of sender
 * @nlhdr: netlink message header
 * @genlhdr: generic netlink message header
 * @userhdr: user specific header
 * @attrs: netlink attributes
 * @_net: network namespace
 * @user_ptr: user pointers
 * @extack: extended ACK report struct
 */
struct genl_info {
	u32			snd_seq;
	u32			snd_portid;
	struct nlmsghdr *	nlhdr;
	struct genlmsghdr *	genlhdr;
	void *			userhdr;
	struct nlattr **	attrs;
	possible_net_t		_net;
	void *			user_ptr[2];
	struct netlink_ext_ack *extack;
};

static inline struct net *genl_info_net(struct genl_info *info)
{
	return read_pnet(&info->_net);
}

static inline void genl_info_net_set(struct genl_info *info, struct net *net)
{
	write_pnet(&info->_net, net);
}

#define GENL_SET_ERR_MSG(info, msg) NL_SET_ERR_MSG((info)->extack, msg)

enum genl_validate_flags {
	GENL_DONT_VALIDATE_STRICT		= BIT(0),
	GENL_DONT_VALIDATE_DUMP			= BIT(1),
	GENL_DONT_VALIDATE_DUMP_STRICT		= BIT(2),
};

/**
 * struct genl_small_ops - generic netlink operations (small version)
 * @cmd: command identifier
 * @internal_flags: flags used by the family
 * @flags: flags
 * @validate: validation flags from enum genl_validate_flags
 * @doit: standard command callback
 * @dumpit: callback for dumpers
 *
 * This is a cut-down version of struct genl_ops for users who don't need
 * most of the ancillary infra and want to save space.
 */
struct genl_small_ops {
	int	(*doit)(struct sk_buff *skb, struct genl_info *info);
	int	(*dumpit)(struct sk_buff *skb, struct netlink_callback *cb);
	u8	cmd;
	u8	internal_flags;
	u8	flags;
	u8	validate;
};

/**
 * struct genl_ops - generic netlink operations
 * @cmd: command identifier
 * @internal_flags: flags used by the family
 * @flags: flags
 * @maxattr: maximum number of attributes supported
 * @policy: netlink policy (takes precedence over family policy)
 * @validate: validation flags from enum genl_validate_flags
 * @doit: standard command callback
 * @start: start callback for dumps
 * @dumpit: callback for dumpers
 * @done: completion callback for dumps
 */
struct genl_ops {
	int		       (*doit)(struct sk_buff *skb,
				       struct genl_info *info);
	int		       (*start)(struct netlink_callback *cb);
	int		       (*dumpit)(struct sk_buff *skb,
					 struct netlink_callback *cb);
	int		       (*done)(struct netlink_callback *cb);
	const struct nla_policy *policy;
	unsigned int		maxattr;
	u8			cmd;
	u8			internal_flags;
	u8			flags;
	u8			validate;
};

/**
 * struct genl_info - info that is available during dumpit op call
 * @family: generic netlink family - for internal genl code usage
 * @ops: generic netlink ops - for internal genl code usage
 * @attrs: netlink attributes
 */
struct genl_dumpit_info {
	const struct genl_family *family;
	struct genl_ops op;
	struct nlattr **attrs;
};

static inline const struct genl_dumpit_info *
genl_dumpit_info(struct netlink_callback *cb)
{
	return cb->data;
}

int genl_register_family(struct genl_family *family);
int genl_unregister_family(const struct genl_family *family);
void genl_notify(const struct genl_family *family, struct sk_buff *skb,
		 struct genl_info *info, u32 group, gfp_t flags);

void *genlmsg_put(struct sk_buff *skb, u32 portid, u32 seq,
		  const struct genl_family *family, int flags, u8 cmd);

/**
 * genlmsg_nlhdr - Obtain netlink header from user specified header
 * @user_hdr: user header as returned from genlmsg_put()
 *
 * Returns pointer to netlink header.
 */
static inline struct nlmsghdr *genlmsg_nlhdr(void *user_hdr)
{
	return (struct nlmsghdr *)((char *)user_hdr -
				   GENL_HDRLEN -
				   NLMSG_HDRLEN);
}

/**
 * genlmsg_parse_deprecated - parse attributes of a genetlink message
 * @nlh: netlink message header
 * @family: genetlink message family
 * @tb: destination array with maxtype+1 elements
 * @maxtype: maximum attribute type to be expected
 * @policy: validation policy
 * @extack: extended ACK report struct
 */
static inline int genlmsg_parse_deprecated(const struct nlmsghdr *nlh,
					   const struct genl_family *family,
					   struct nlattr *tb[], int maxtype,
					   const struct nla_policy *policy,
					   struct netlink_ext_ack *extack)
{
	return __nlmsg_parse(nlh, family->hdrsize + GENL_HDRLEN, tb, maxtype,
			     policy, NL_VALIDATE_LIBERAL, extack);
}

/**
 * genlmsg_parse - parse attributes of a genetlink message
 * @nlh: netlink message header
 * @family: genetlink message family
 * @tb: destination array with maxtype+1 elements
 * @maxtype: maximum attribute type to be expected
 * @policy: validation policy
 * @extack: extended ACK report struct
 */
static inline int genlmsg_parse(const struct nlmsghdr *nlh,
				const struct genl_family *family,
				struct nlattr *tb[], int maxtype,
				const struct nla_policy *policy,
				struct netlink_ext_ack *extack)
{
	return __nlmsg_parse(nlh, family->hdrsize + GENL_HDRLEN, tb, maxtype,
			     policy, NL_VALIDATE_STRICT, extack);
}

/**
 * genl_dump_check_consistent - check if sequence is consistent and advertise if not
 * @cb: netlink callback structure that stores the sequence number
 * @user_hdr: user header as returned from genlmsg_put()
 *
 * Cf. nl_dump_check_consistent(), this just provides a wrapper to make it
 * simpler to use with generic netlink.
 */
static inline void genl_dump_check_consistent(struct netlink_callback *cb,
					      void *user_hdr)
{
	nl_dump_check_consistent(cb, genlmsg_nlhdr(user_hdr));
}

/**
 * genlmsg_put_reply - Add generic netlink header to a reply message
 * @skb: socket buffer holding the message
 * @info: receiver info
 * @family: generic netlink family
 * @flags: netlink message flags
 * @cmd: generic netlink command
 *
 * Returns pointer to user specific header
 */
static inline void *genlmsg_put_reply(struct sk_buff *skb,
				      struct genl_info *info,
				      const struct genl_family *family,
				      int flags, u8 cmd)
{
	return genlmsg_put(skb, info->snd_portid, info->snd_seq, family,
			   flags, cmd);
}

/**
 * genlmsg_end - Finalize a generic netlink message
 * @skb: socket buffer the message is stored in
 * @hdr: user specific header
 */
static inline void genlmsg_end(struct sk_buff *skb, void *hdr)
{
	nlmsg_end(skb, hdr - GENL_HDRLEN - NLMSG_HDRLEN);
}

/**
 * genlmsg_cancel - Cancel construction of a generic netlink message
 * @skb: socket buffer the message is stored in
 * @hdr: generic netlink message header
 */
static inline void genlmsg_cancel(struct sk_buff *skb, void *hdr)
{
	if (hdr)
		nlmsg_cancel(skb, hdr - GENL_HDRLEN - NLMSG_HDRLEN);
}

/**
 * genlmsg_multicast_netns - multicast a netlink message to a specific netns
 * @family: the generic netlink family
 * @net: the net namespace
 * @skb: netlink message as socket buffer
 * @portid: own netlink portid to avoid sending to yourself
 * @group: offset of multicast group in groups array
 * @flags: allocation flags
 */
static inline int genlmsg_multicast_netns(const struct genl_family *family,
					  struct net *net, struct sk_buff *skb,
					  u32 portid, unsigned int group, gfp_t flags)
{
	if (WARN_ON_ONCE(group >= family->n_mcgrps))
		return -EINVAL;
	group = family->mcgrp_offset + group;
	return nlmsg_multicast(net->genl_sock, skb, portid, group, flags);
}

/**
 * genlmsg_multicast - multicast a netlink message to the default netns
 * @family: the generic netlink family
 * @skb: netlink message as socket buffer
 * @portid: own netlink portid to avoid sending to yourself
 * @group: offset of multicast group in groups array
 * @flags: allocation flags
 */
static inline int genlmsg_multicast(const struct genl_family *family,
				    struct sk_buff *skb, u32 portid,
				    unsigned int group, gfp_t flags)
{
	return genlmsg_multicast_netns(family, &init_net, skb,
				       portid, group, flags);
}

/**
 * genlmsg_multicast_allns - multicast a netlink message to all net namespaces
 * @family: the generic netlink family
 * @skb: netlink message as socket buffer
 * @portid: own netlink portid to avoid sending to yourself
 * @group: offset of multicast group in groups array
 * @flags: allocation flags
 *
 * This function must hold the RTNL or rcu_read_lock().
 */
int genlmsg_multicast_allns(const struct genl_family *family,
			    struct sk_buff *skb, u32 portid,
			    unsigned int group, gfp_t flags);

/**
 * genlmsg_unicast - unicast a netlink message
 * @skb: netlink message as socket buffer
 * @portid: netlink portid of the destination socket
 */
static inline int genlmsg_unicast(struct net *net, struct sk_buff *skb, u32 portid)
{
	return nlmsg_unicast(net->genl_sock, skb, portid);
}

/**
 * genlmsg_reply - reply to a request
 * @skb: netlink message to be sent back
 * @info: receiver information
 */
static inline int genlmsg_reply(struct sk_buff *skb, struct genl_info *info)
{
	return genlmsg_unicast(genl_info_net(info), skb, info->snd_portid);
}

/**
 * gennlmsg_data - head of message payload
 * @gnlh: genetlink message header
 */
static inline void *genlmsg_data(const struct genlmsghdr *gnlh)
{
	return ((unsigned char *) gnlh + GENL_HDRLEN);
}

/**
 * genlmsg_len - length of message payload
 * @gnlh: genetlink message header
 */
static inline int genlmsg_len(const struct genlmsghdr *gnlh)
{
	struct nlmsghdr *nlh = (struct nlmsghdr *)((unsigned char *)gnlh -
							NLMSG_HDRLEN);
	return (nlh->nlmsg_len - GENL_HDRLEN - NLMSG_HDRLEN);
}

/**
 * genlmsg_msg_size - length of genetlink message not including padding
 * @payload: length of message payload
 */
static inline int genlmsg_msg_size(int payload)
{
	return GENL_HDRLEN + payload;
}

/**
 * genlmsg_total_size - length of genetlink message including padding
 * @payload: length of message payload
 */
static inline int genlmsg_total_size(int payload)
{
	return NLMSG_ALIGN(genlmsg_msg_size(payload));
}

/**
 * genlmsg_new - Allocate a new generic netlink message
 * @payload: size of the message payload
 * @flags: the type of memory to allocate.
 */
static inline struct sk_buff *genlmsg_new(size_t payload, gfp_t flags)
{
	return nlmsg_new(genlmsg_total_size(payload), flags);
}

/**
 * genl_set_err - report error to genetlink broadcast listeners
 * @family: the generic netlink family
 * @net: the network namespace to report the error to
 * @portid: the PORTID of a process that we want to skip (if any)
 * @group: the broadcast group that will notice the error
 * 	(this is the offset of the multicast group in the groups array)
 * @code: error code, must be negative (as usual in kernelspace)
 *
 * This function returns the number of broadcast listeners that have set the
 * NETLINK_RECV_NO_ENOBUFS socket option.
 */
static inline int genl_set_err(const struct genl_family *family,
			       struct net *net, u32 portid,
			       u32 group, int code)
{
	if (WARN_ON_ONCE(group >= family->n_mcgrps))
		return -EINVAL;
	group = family->mcgrp_offset + group;
	return netlink_set_err(net->genl_sock, portid, group, code);
}

static inline int genl_has_listeners(const struct genl_family *family,
				     struct net *net, unsigned int group)
{
	if (WARN_ON_ONCE(group >= family->n_mcgrps))
		return -EINVAL;
	group = family->mcgrp_offset + group;
	return netlink_has_listeners(net->genl_sock, group);
}

////add for cypress John_gao 


static inline void __bp_genl_info_userhdr_set(struct genl_info *info,
					      void *userhdr)
{
	info->userhdr = userhdr;
}

static inline void *__bp_genl_info_userhdr(struct genl_info *info)
{
	return info->userhdr;
}

#if LINUX_VERSION_IS_LESS(4,12,0)
#define GENL_SET_ERR_MSG(info, msg) NL_SET_ERR_MSG(genl_info_extack(info), msg)

static inline int genl_err_attr(struct genl_info *info, int err,
				struct nlattr *attr)
{
	return err;
}
#endif /* < 4.12 */

/* this is for patches we apply */
static inline struct netlink_ext_ack *genl_info_extack(struct genl_info *info)
{
#if LINUX_VERSION_IS_GEQ(4,12,0)
	return info->extack;
#else
	return info->userhdr;
#endif
}

/* this is for patches we apply */
static inline struct netlink_ext_ack *genl_callback_extack(struct netlink_callback *cb)
{
#if LINUX_VERSION_IS_GEQ(4,20,0)
	return cb->extack;
#else
	return NULL;
#endif
}

/* this gets put in place of info->userhdr, since we use that above */
static inline void *genl_info_userhdr(struct genl_info *info)
{
	return (u8 *)info->genlhdr + GENL_HDRLEN;
}

/* this is for patches we apply */
#if LINUX_VERSION_IS_LESS(3,7,0)
#define genl_info_snd_portid(__genl_info) (__genl_info->snd_pid)
#else
#define genl_info_snd_portid(__genl_info) (__genl_info->snd_portid)
#endif

#if LINUX_VERSION_IS_LESS(3,13,0)
#define __genl_const
#else /* < 3.13 */
#define __genl_const const
#endif /* < 3.13 */

#ifndef GENLMSG_DEFAULT_SIZE
#define GENLMSG_DEFAULT_SIZE (NLMSG_DEFAULT_SIZE - GENL_HDRLEN)
#endif

#if LINUX_VERSION_IS_LESS(3,1,0)
#define genl_dump_check_consistent(cb, user_hdr) do { } while (0)
#endif

#if LINUX_VERSION_IS_LESS(4,10,0)
#define __genl_ro_after_init
#else
#define __genl_ro_after_init __ro_after_init
#endif

#if LINUX_VERSION_IS_LESS(4,15,0)
#define genlmsg_nlhdr LINUX_BACKPORT(genlmsg_nlhdr)
static inline struct nlmsghdr *genlmsg_nlhdr(void *user_hdr)
{
	return (struct nlmsghdr *)((char *)user_hdr -
				   GENL_HDRLEN -
				   NLMSG_HDRLEN);
}

#ifndef genl_dump_check_consistent
static inline
void backport_genl_dump_check_consistent(struct netlink_callback *cb,
					 void *user_hdr)
{
	struct genl_family dummy_family = {
		.hdrsize = 0,
	};

	genl_dump_check_consistent(cb, user_hdr, &dummy_family);
}
#define genl_dump_check_consistent LINUX_BACKPORT(genl_dump_check_consistent)
#endif
#endif /* LINUX_VERSION_IS_LESS(4,15,0) */

#if LINUX_VERSION_IS_LESS(5,2,0)
enum genl_validate_flags {
	GENL_DONT_VALIDATE_STRICT		= BIT(0),
	GENL_DONT_VALIDATE_DUMP			= BIT(1),
	GENL_DONT_VALIDATE_DUMP_STRICT		= BIT(2),
};

#if LINUX_VERSION_IS_GEQ(3,13,0)
struct backport_genl_ops {
	void			*__dummy_was_policy_must_be_null;
	int		       (*doit)(struct sk_buff *skb,
				       struct genl_info *info);
#if LINUX_VERSION_IS_GEQ(4,5,0) || \
    LINUX_VERSION_IN_RANGE(4,4,104, 4,5,0) || \
    LINUX_VERSION_IN_RANGE(4,1,48, 4,2,0) || \
    LINUX_VERSION_IN_RANGE(3,18,86, 3,19,0)
	int		       (*start)(struct netlink_callback *cb);
#endif
	int		       (*dumpit)(struct sk_buff *skb,
					 struct netlink_callback *cb);
	int		       (*done)(struct netlink_callback *cb);
	u8			cmd;
	u8			internal_flags;
	u8			flags;
	u8			validate;
};
#else
struct backport_genl_ops {
	u8			cmd;
	u8			internal_flags;
	unsigned int		flags;
	void			*__dummy_was_policy_must_be_null;
	int		       (*doit)(struct sk_buff *skb,
				       struct genl_info *info);
	int		       (*dumpit)(struct sk_buff *skb,
					 struct netlink_callback *cb);
	int		       (*done)(struct netlink_callback *cb);
	struct list_head	ops_list;
	u8			validate;
};
#endif

static inline int
__real_backport_genl_register_family(struct genl_family *family)
{
#define OPS_VALIDATE(f) \
	BUILD_BUG_ON(offsetof(struct genl_ops, f) != \
		     offsetof(struct backport_genl_ops, f))
	OPS_VALIDATE(doit);
#if LINUX_VERSION_IS_GEQ(4,5,0) || \
    LINUX_VERSION_IN_RANGE(4,4,104, 4,5,0) || \
    LINUX_VERSION_IN_RANGE(4,1,48, 4,2,0) || \
    LINUX_VERSION_IN_RANGE(3,18,86, 3,19,0)
	OPS_VALIDATE(start);
#endif
	OPS_VALIDATE(dumpit);
	OPS_VALIDATE(done);
	OPS_VALIDATE(cmd);
	OPS_VALIDATE(internal_flags);
	OPS_VALIDATE(flags);

	return genl_register_family(family);
}
#define genl_ops backport_genl_ops

static inline int
__real_backport_genl_unregister_family(struct genl_family *family)
{
	return genl_unregister_family(family);
}

struct backport_genl_family {
	struct genl_family	family;
	const struct genl_ops *	copy_ops;

	/* copied */
	int			id;		/* private */
	unsigned int		hdrsize;
	char			name[GENL_NAMSIZ];
	unsigned int		version;
	unsigned int		maxattr;
	bool			netnsok;
	bool			parallel_ops;
	const struct nla_policy *policy;
	int			(*pre_doit)(__genl_const struct genl_ops *ops,
					    struct sk_buff *skb,
					    struct genl_info *info);
	void			(*post_doit)(__genl_const struct genl_ops *ops,
					     struct sk_buff *skb,
					     struct genl_info *info);
/*
 * unsupported!
	int			(*mcast_bind)(struct net *net, int group);
	void			(*mcast_unbind)(struct net *net, int group);
 */
	struct nlattr **	attrbuf;	/* private */
	__genl_const struct genl_ops *	ops;
	__genl_const struct genl_multicast_group *mcgrps;
	unsigned int		n_ops;
	unsigned int		n_mcgrps;
	struct module		*module;
};
#undef genl_family
#define genl_family backport_genl_family

#define genl_register_family backport_genl_register_family
int genl_register_family(struct genl_family *family);

#define genl_unregister_family backport_genl_unregister_family
int backport_genl_unregister_family(struct genl_family *family);

#define genl_notify LINUX_BACKPORT(genl_notify)
void genl_notify(const struct genl_family *family, struct sk_buff *skb,
		 struct genl_info *info, u32 group, gfp_t flags);

#define genlmsg_put LINUX_BACKPORT(genlmsg_put)
void *genlmsg_put(struct sk_buff *skb, u32 portid, u32 seq,
		  const struct genl_family *family, int flags, u8 cmd);

#define genlmsg_put_reply LINUX_BACKPORT(genlmsg_put_reply)
void *genlmsg_put_reply(struct sk_buff *skb,
			struct genl_info *info,
			const struct genl_family *family,
			int flags, u8 cmd);

#define genlmsg_multicast_netns LINUX_BACKPORT(genlmsg_multicast_netns)
int genlmsg_multicast_netns(const struct genl_family *family,
			    struct net *net, struct sk_buff *skb,
			    u32 portid, unsigned int group,
			    gfp_t flags);

#define genlmsg_multicast LINUX_BACKPORT(genlmsg_multicast)
int genlmsg_multicast(const struct genl_family *family,
		      struct sk_buff *skb, u32 portid,
		      unsigned int group, gfp_t flags);

#define genlmsg_multicast_allns LINUX_BACKPORT(genlmsg_multicast_allns)
int backport_genlmsg_multicast_allns(const struct genl_family *family,
				     struct sk_buff *skb, u32 portid,
				     unsigned int group, gfp_t flags);

#define genl_family_attrbuf LINUX_BACKPORT(genl_family_attrbuf)
static inline struct nlattr **genl_family_attrbuf(struct genl_family *family)
{
	WARN_ON(family->parallel_ops);

	return family->attrbuf;
}

#define genlmsg_parse LINUX_BACKPORT(genlmsg_parse)
static inline int genlmsg_parse(const struct nlmsghdr *nlh,
				const struct genl_family *family,
				struct nlattr *tb[], int maxtype,
				const struct nla_policy *policy,
				struct netlink_ext_ack *extack)
{
	return __nlmsg_parse(nlh, family->hdrsize + GENL_HDRLEN, tb, maxtype,
			     policy, NL_VALIDATE_STRICT, extack);
}
#endif /* LINUX_VERSION_IS_LESS(5,2,0) */

#endif	/* __NET_GENERIC_NETLINK_H */
