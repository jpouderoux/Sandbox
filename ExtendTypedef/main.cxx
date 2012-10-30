#include "Functor.h"
#include "Modify.h"

 #include <boost/mpl/assert.hpp>
#include <boost/type_traits/is_same.hpp>

int main()
{
  typedef Modify<functor::Derived,arg::Replace> ModifiedType;
  typedef GetTypes<ModifiedType::ExecutionSignature> ETypes;
  typedef GetTypes<ModifiedType::ControlSignature> CTypes;

  //confirm that we inserted _2 to execution signature
  BOOST_MPL_ASSERT(( boost::is_same<ETypes::Arg2Type,arg::Arg<2> > ));

  //confirm that we insterted the InsertedArg type to control signature
  BOOST_MPL_ASSERT(( boost::is_same<CTypes::Arg2Type,arg::InsertedArg > ));


  //next step is to convert the boost mpl types back to a worklet
  //signature. I have a feeling we will need the boost preprocessor
  //for this work
}
