#ifndef POLICY_TYPES_HPP
#define POLICY_TYPES_HPP

namespace type_builder{

/**
 * @brief This class is the default base class for number_types that don't need a base.
 *
 * If default-initializiation is enabled, all numbers will be initialized with 0.
 * 
 * NEVER use this class to refer to a type_builder::basic_number as it has no virtual
 * destructor and will blow away all type-safety you would gain otherwise.
 */
template<typename T, typename Tid>
struct empty_base{
	static constexpr T default_value(){
		return T{};
	}
	
	enum: bool{
		USE_DEFAULT_STREAM_IN = true,
		USE_DEFAULT_STREAM_OUT = true
	};
	
};

} //namespace type_builder

#endif
