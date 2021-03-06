// -*- C++ -*-
//
// $Id: Messaging_Policy_i.inl 73791 2006-07-27 20:54:56Z wotte $

#if (TAO_HAS_SYNC_SCOPE_POLICY == 1)

TAO_BEGIN_VERSIONED_NAMESPACE_DECL

ACE_INLINE void
TAO_Sync_Scope_Policy::get_synchronization (Messaging::SyncScope &s) const
{
  s = this->synchronization_;
}

TAO_END_VERSIONED_NAMESPACE_DECL

#endif /* TAO_HAS_SYNC_SCOPE_POLICY == 1 */
