#ifndef __params_flatten_h
#define __params_flatten_h

#include "common.h"

#include <boost/fusion/algorithm/transformation/insert_range.hpp>
#include <boost/fusion/algorithm/transformation/push_back.hpp>
#include <boost/fusion/functional/invocation/invoke_procedure.hpp>
#include <boost/fusion/iterator/deref.hpp>
#include <boost/fusion/iterator/next.hpp>
#include <boost/fusion/support/is_sequence.hpp>
#include <boost/fusion/support/is_view.hpp>
#include <boost/mpl/or.hpp>
#include <boost/type_traits/remove_reference.hpp>

//work around a bug in boost::fusion::is_view in that it
//doesn't properly implement the non_fusion_tag as meaning something isn't
//a view
namespace boost { namespace fusion { namespace extension {
  template <>
  struct is_view_impl<non_fusion_tag>
  {
      template <typename Sequence>
      struct apply : mpl::false_ {};
  };
}}}

namespace params {
  namespace detail {

  namespace fusion = ::boost::fusion;

  template<class I>
  struct is_fusion_sequence
  {
    //deref I as it is an iterator
    typedef typename fusion::result_of::deref<I>::type derefType;

    //the iterator might be to a sequence or view reference, which
    //doesn't compile, so we have to remove the reference
    typedef typename boost::remove_reference<derefType>::type deducedType;

    //check if it is a sequence or view
    typedef typename fusion::traits::is_sequence<deducedType>::type isSequenceType;
    typedef typename fusion::traits::is_view<deducedType>::type isViewType;
    typedef typename boost::mpl::or_< isSequenceType, isViewType >::type IsIterator;
  public:
    typedef IsIterator type;
  };


  template< int Size, int Element, class Functor>
  struct flatten_impl
  {
    template<class Item, class Sequence>
    void operator()(Functor& f, Item item, Sequence s,
                    typename boost::enable_if<
                    typename params::detail::is_fusion_sequence<Item>::type >::type* = 0) const
    {
      //insert every element in the sequence contained in item to the end
      //of the Sequence s.
      flatten_impl<Size,Element+1,Functor>()(f,
                 fusion::next(item),
                 fusion::insert_range(s,fusion::end(s),fusion::deref(item)));
    }

    template<class Item, class Sequence>
    void operator()(Functor& f, Item item, Sequence s,
                    typename boost::disable_if<
                    typename params::detail::is_fusion_sequence<Item>::type >::type* = 0) const
    {
      //push the derefence of the item onto the end of sequence s
      flatten_impl<Size,Element+1,Functor>()(f,
                                   fusion::next(item),
                                   fusion::push_back(s,fusion::deref(item)));
    }
  };

  //termination case of the flatten recursive calls
  template< int Size, class Functor>
  struct flatten_impl<Size, Size, Functor>
  {
    template<class Item, class Sequence>
    void operator()(Functor& f, Item, Sequence s) const
    {
      //item is an iterator pointing to end, so everything is in sequence
    fusion::invoke_procedure(f,s);
    }
  };

  //function that starts the flatten recursion when the first item is a sequence
  template< int Size, class Functor, class Item>
  void flatten(Functor& f, Item item,
               typename boost::enable_if<
               typename params::detail::is_fusion_sequence<Item>::type >::type* = 0)
  {
    params::detail::flatten_impl<Size,1,Functor>()(f,fusion::next(item),
                                                     fusion::deref(item));
  }

  //function that starts the flatten recursion when the first item isn't a sequence
  template< int Size, class Functor, class Item>
  void flatten(Functor& f, Item item,
               typename boost::disable_if<
               typename params::detail::is_fusion_sequence<Item>::type >::type* = 0)
  {
    typedef typename fusion::result_of::deref<Item>::type ItemDeRef;
    typedef fusion::single_view<ItemDeRef> ItemView;
    ItemView iv(fusion::deref(item));
    params::detail::flatten_impl<Size,1,Functor>()(f,fusion::next(item),iv);
  }

} } //params::detail


namespace params
{
  //take an arbitrary class that has a parameter pack and flatten it so
  //that we can call a method with each element of the class
  template< class Functor,
            class ... Args>
  void flatten(Functor& f, Args... args)
  {
    typedef typename ::params::Join< ::params::vector,Args... >::type Sequence;
    Sequence all_args(args...);

    ::params::detail::flatten<
      ::params::detail::num_elements<Sequence>::value,
      Functor>(f,boost::fusion::begin(all_args));
  }

}
#endif