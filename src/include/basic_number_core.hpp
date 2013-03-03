#ifndef BASIC_NUMBER_CORE_HPP
#define BASIC_NUMBER_CORE_HPP

#include <utility>
#include <cstdint>
#include <type_traits>

#include "basic_number_flags.hpp"
#include "policy_types.hpp"

// to check for broken access-controll in gcc:
#ifdef __GNUC__ 
#	if __GNUC__ <= 4 && __GNUC_MINOR__ < 8
#		ifndef __llvm__ // clang LIES to us, so we need to test for it too...
#			warning "compiling with broken gcc; the value in basic_number will be public"
#			define BROKEN_GCC_VERSION
#		endif
#	endif
#endif

namespace type_builder{

/**
 * @brief A template for a number-class.
 */
// note that the operators are not documented with doxygen, 
// because they behave pretty much as you would expect anyway.
template<
	typename T,
	class Tid,
	flag_t Tflags = DEFAULT_SETTINGS,
	template<typename, class> class Tbase = empty_base>
class basic_number: public Tbase<T, Tid> {
	//put the private stuff to the begining because we will need it in 
	// signatures:
	
	/**
	 * @brief The value of the number.
	 */
#ifdef BROKEN_GCC_VERSION // needed for usage of decltype in signatures
	public:
#endif
	T value;
#ifdef BROKEN_GCC_VERSION 
	private:
#endif
	
	/**
	 * @brief Checks if a type is identical to *this.
	 * @return true if it is, false otherwise.
	 */
	template<typename Targ> struct is_this{ 
		enum : bool{
			value = ( std::is_same<const basic_number&, const Targ&>::value 
				|| std::is_same<basic_number&, Targ&>::value)
		};
	};
	
	static_assert(is_this<basic_number>::value, "is_this doesn't work.");
	static_assert(!is_this<basic_number<T,basic_number>>::value, "is_this doesn't work.");
	static_assert(!is_this<int>::value, "is_this doesn't work.");
	
	/**
	 * @brief Provides the correct returntype for some functions.
	 * @param T1 the return type if native typing is enabled
	 * @param T2 the return type if native typing is disabled
	 */
	template<typename T1, typename T2>
	struct return_type{
		using type = typename std::conditional<(Tflags & ENABLE_NATIVE_TYPING) != 0, T1, T2>::type;
	};
	
	// implementation for is_equivalent_basic_number
	template<typename Targ>
	struct _is_equivalent_basic_number{
		enum : bool{
			value = 0
		};
	};
	// dito:
	template<typename Targ>
	struct _is_equivalent_basic_number<basic_number<Targ, Tid, Tflags, Tbase>&>{
		enum : bool{
			value = (Tflags & ENABLE_NATIVE_TYPING) ? true : is_this<basic_number<Targ, Tid, Tflags, Tbase>>::value
		};
	};
	
	/**
	 * @brief Checks whether a type is semanticly equivalent to *this.
	 * @param Targ the type that will be checked.
	 * @return true if it is equivalent, false otherwise
	 */
	template<typename Targ>
	struct is_equivalent_basic_number{
		enum : bool{
			value = _is_equivalent_basic_number<
				typename std::remove_const<
					typename std::remove_reference<Targ>::type
				>::type&>::value
		};
	};
	
	static_assert(is_equivalent_basic_number<basic_number>::value, 
		"is_equivalent_basic_number doesn't work.");
	static_assert(is_equivalent_basic_number<basic_number&>::value, 
		"is_equivalent_basic_number doesn't work.");
	static_assert(is_equivalent_basic_number<const basic_number>::value, 
		"is_equivalent_basic_number doesn't work.");
	static_assert(is_equivalent_basic_number<const basic_number&>::value, 
		"is_equivalent_basic_number doesn't work.");
	static_assert(is_equivalent_basic_number<basic_number&&>::value, 
		"is_equivalent_basic_number doesn't work.");
	
	/**
	 * @brief Checks whether a flag is set.
	 * @param Tflag the flag to be checked
	 * @return true if Tflag is set, false otherwise
	 */
	template<flag_t Tflag>
	struct flag_set{
		enum{
			value = (Tflags & Tflag) != 0 ? true : false
		};
	};
	
	/**
	 * @brief Checks whether a flag is unset.
	 * @param Tflag the flag to be checked
	 * @return true if Tflag is unset, false otherwise
	 */
	template<flag_t Tflag>
	struct flag_unset{
		enum{
			value = ! flag_set<Tflag>::value
		};
	};
	
	public:
		
		// constructors:
		
		// clang says I cannot disable this constructor with std::enable_if :-(
		basic_number() : value(Tbase<T, Tid>::default_value()) {
			static_assert( flag_unset<DISABLE_CONSTRUCTION>::value, 
					"construction of this type is disabled");
			static_assert( flag_set<ENABLE_DEFAULT_CONSTRUCTION>::value,
					"default constructor not enabled for this type");
		}
		
		// basic_number(T)
		template<typename Tother>
		explicit basic_number(Tother&& value,
			typename std::enable_if<(std::is_same<Tother&, T&>::value)
				&& flag_unset<DISABLE_CONSTRUCTION>::value>::type* = 0
		): value(std::forward<T>(value)){}
		
		// basic_number(basic_number)
		template<typename Tother>
		basic_number(Tother&& other, 
			typename std::enable_if<(is_equivalent_basic_number<Tother>::value) 
				&& flag_unset<DISABLE_CONSTRUCTION>::value>::type* = 0
		): value(std::forward<T>(other.get_value())){}
		
		// basic_number(Tother)
		template<typename Tother>
		explicit basic_number(Tother&& value,
			typename std::enable_if<
				(!is_equivalent_basic_number<Tother>::value) 
				&& (!std::is_same<Tother&, T&>::value)
				&& flag_unset<DISABLE_CONSTRUCTION>::value
				&& flag_unset<ENABLE_GENERAL_CONSTRUCTION>::value>::type* = 0
		): value(std::forward<T>(value)){}
		
		// methods:
		
		// *this = basic_number
		template<typename Tother> 
		typename std::enable_if<
			is_equivalent_basic_number<Tother>::value
			&& flag_unset<DISABLE_MUTABILITY>::value,
		basic_number&>::type
		operator=(Tother&& other){
			// we should check for self-assignement:
			if(this != &other){
				this->value = std::forward<Tother>(other).value;
			}
			return *this;
		}
		
		// *this = Tother
		template<typename Tother>
		typename std::enable_if< 
			!is_equivalent_basic_number<Tother>::value
			&& !(std::is_same<T&, Tother&>::value)
			&& flag_set<ENABLE_LATE_ASSIGNEMENT>::value
			&& flag_set<ENABLE_GENERAL_CONSTRUCTION>::value,
		basic_number&>::type
		operator=(Tother&& value){
			this->value = std::forward<Tother>(value);
			return *this;
		}
		
		// *this == basic_number
		template <typename Tother>
		typename std::enable_if< is_equivalent_basic_number<Tother>::value
			&& flag_set<ENABLE_SPECIFIC_EQUALITY_CHECK>::value, bool>::type
		operator==(Tother&& other) const{
			return value == other.get_value();
		}
		
		// *this == Tother
		template <typename Tother>
		typename std::enable_if< !(is_equivalent_basic_number<Tother>::value)
			&& flag_set<ENABLE_EQUALITY_CHECK_O_>::value, bool>::type
		operator==(Tother&& other) const{
			return value == other;
		}
		
		// *this != basic_number
		template <typename Tother>
		typename std::enable_if< is_equivalent_basic_number<Tother>::value
			&& flag_set<ENABLE_SPECIFIC_EQUALITY_CHECK>::value, bool>::type
		operator!=(Tother&& other) const{
			return value != other.get_value();
		}
		
		// *this != Tother
		template <typename Tother>
		typename std::enable_if< !(is_equivalent_basic_number<Tother>::value)
			&& flag_set<ENABLE_EQUALITY_CHECK_O_>::value, bool>::type
		operator!=(Tother&& other) const{
			return value != other;
		}
		
		// *this < basic_number
		template <typename Tother>
		typename std::enable_if< is_equivalent_basic_number<Tother>::value
			&& flag_set<ENABLE_SPECIFIC_ORDERING_O_>::value, bool>::type
		operator<(Tother&& other) const{
			return value < other.get_value();
		}
		
		// *this < Tother
		template <typename Tother>
		typename std::enable_if< !(is_equivalent_basic_number<Tother>::value)
			&& flag_set<ENABLE_ORDERING_O_>::value, bool>::type
		operator<(Tother&& other) const{
			return value < other;
		}
		
		// *this <= basic_number
		template <typename Tother>
		typename std::enable_if< is_equivalent_basic_number<Tother>::value
			&& flag_set<ENABLE_SPECIFIC_ORDERING_O_>::value, bool>::type
		operator<=(Tother&& other) const{
			return value <= other.get_value();
		}
		
		// *this <= Tother
		template <typename Tother>
		typename std::enable_if< !(is_equivalent_basic_number<Tother>::value)
			&& flag_set<ENABLE_ORDERING_O_>::value, bool>::type
		operator<=(Tother&& other) const{
			return value <= other;
		}
		
		// *this > basic_number
		template <typename Tother>
		typename std::enable_if< is_equivalent_basic_number<Tother>::value
			&& flag_set<ENABLE_SPECIFIC_ORDERING_O_>::value, bool>::type
		operator>(Tother&& other) const{
			return value > other.get_value();
		}
		
		// *this > Tother
		template <typename Tother>
		typename std::enable_if< !(is_equivalent_basic_number<Tother>::value)
			&& flag_set<ENABLE_ORDERING_O_>::value, bool>::type
		operator>(Tother&& other) const{
			return value > other;
		}
		
		
		// *this >= basic_number
		template <typename Tother>
		typename std::enable_if< is_equivalent_basic_number<Tother>::value
			&& flag_set<ENABLE_SPECIFIC_ORDERING_O_>::value, bool>::type
		operator>=(Tother&& other) const{
			return value >= other.get_value();
		}
		
		// *this >= Tother
		template <typename Tother>
		typename std::enable_if< !(is_equivalent_basic_number<Tother>::value)
			&& flag_set<ENABLE_ORDERING_O_>::value, bool>::type
		operator>=(Tother&& other) const{
			return value >= other;
		}
		
		basic_number& operator++(){
			static_assert( !(Tflags & DISABLE_MUTABILITY),
					"You cannot change the value of an instance "
					"of an immutable type");
			static_assert(Tflags & ENABLE_INC_DEC,
					"increment not enabled for this number-type");
			++value;
			return *this;
		}
		
		basic_number operator++(int){
			static_assert( !(Tflags & DISABLE_MUTABILITY),
					"You cannot change the value of an instance "
					"of an immutable type");
			static_assert(Tflags & ENABLE_INC_DEC,
					"increment not enabled for this number-type");
			return basic_number(value++);
		}
		
		basic_number& operator--(){
			static_assert( !(Tflags & DISABLE_MUTABILITY),
					"You cannot change the value of an instance "
					"of an immutable type");
			static_assert(Tflags & ENABLE_INC_DEC,
					"increment not enabled for this number-type");
			--value;
			return *this;
		}
		
		basic_number operator--(int){
			static_assert( !(Tflags & DISABLE_MUTABILITY),
					"You cannot change the value of an instance "
					"of an immutable type");
			static_assert(Tflags & ENABLE_INC_DEC,
					"increment not enabled for this number-type");
			return basic_number(value--);
		}
		
		
		// *this + basic_number
		template<typename Tother>
		auto operator+(Tother&& other) const ->
		typename std::enable_if< is_equivalent_basic_number<Tother>::value
			&& flag_set<ENABLE_SPECIFIC_PLUS_MINUS>::value, 
			typename return_type<
				basic_number<
					decltype(value + other.get_value()), Tid, Tflags,
					Tbase>, 
				basic_number>::type>::type
		{
			using return_type_base = typename return_type<decltype(value + other.get_value()), T>::type;
			return basic_number<return_type_base, Tid, Tflags, Tbase>{
					static_cast<return_type_base>(value + other.get_value())
				};
		}
		
		// *this + Tother
		template<typename Tother>
		auto operator+(Tother&& other) const ->
		typename std::enable_if< !(is_equivalent_basic_number<Tother>::value)
			&& flag_set<ENABLE_GENERAL_PLUS_MINUS>::value, 
			typename return_type< 
				basic_number<decltype(value + other), Tid, Tflags, Tbase>, 
				basic_number>::type >::type
		{
			using return_type_base = typename return_type<decltype(value + other), T>::type;
			return basic_number<return_type_base, Tid, Tflags, Tbase>{
					static_cast<return_type_base>(value + other)
			};
		}
		
		// *this += basic_number
		template<typename Tother>
		typename std::enable_if< is_equivalent_basic_number<Tother>::value
			&& flag_unset<DISABLE_MUTABILITY>::value
			&& flag_set<ENABLE_SPECIFIC_PLUS_MINUS>::value, basic_number&>::type
		operator+=(Tother&& other){
			value += other.get_value();
			return *this;
		}
		
		// *this += Tother
		template<typename Tother>
		typename std::enable_if< !is_equivalent_basic_number<Tother>::value
			&& flag_unset<DISABLE_MUTABILITY>::value
			&& flag_set<ENABLE_GENERAL_PLUS_MINUS>::value, basic_number&>::type
		operator+=(const Tother& other){
			value += other;
			return *this;
		}
		
		// *this - basic_number
		template<typename Tother>
		auto operator-(Tother&& other) const ->
		typename std::enable_if< is_equivalent_basic_number<Tother>::value
			&& flag_set<ENABLE_SPECIFIC_PLUS_MINUS>::value, 
			typename return_type<
				basic_number<decltype(value - other.get_value()), Tid, Tflags, Tbase>, 
				basic_number>::type>::type
		{
			using return_type_base = typename return_type<decltype(value - other.get_value()), T>::type;
			return basic_number<return_type_base, Tid, Tflags, Tbase>{
					static_cast<return_type_base>(value - other.get_value())
				};
		}
		
		// *this - Tother
		template<typename Tother>
		auto operator-(Tother&& other) const ->
		typename std::enable_if< !(is_equivalent_basic_number<Tother>::value)
			&& flag_set<ENABLE_GENERAL_PLUS_MINUS>::value, 
			typename return_type< 
				basic_number<decltype(value - other), Tid, Tflags, Tbase>, 
				basic_number>::type >::type
		{
			using return_type_base = typename return_type<decltype(value - other), T>::type;
			return basic_number<return_type_base, Tid, Tflags, Tbase>{
					static_cast<return_type_base>(value - other)
				};
		}
		
		// *this -= basic_number
		template<typename Tother>
		typename std::enable_if< is_equivalent_basic_number<Tother>::value
			&& flag_unset<DISABLE_MUTABILITY>::value
			&& flag_set<ENABLE_SPECIFIC_PLUS_MINUS>::value, basic_number&>::type
		operator-=(Tother&& other){
			value -= other.get_value();
			return *this;
		}
		
		// *this -= Tother
		template<typename Tother>
		typename std::enable_if< !is_equivalent_basic_number<Tother>::value
			&& flag_unset<Tflags & DISABLE_MUTABILITY>::value
			&& flag_set<Tflags & ENABLE_GENERAL_PLUS_MINUS>::value, basic_number&>::type
		operator-=(const Tother& other){
			value -= other;
			return *this;
		}
		
		// *this * basic_number
		template<typename Tother>
		auto operator*(Tother&& other) const ->
		typename std::enable_if< is_equivalent_basic_number<Tother>::value
			&& flag_set<ENABLE_SPECIFIC_MULTIPLICATION>::value,
			typename return_type<
				basic_number<decltype(value - other.get_value()), Tid, Tflags, Tbase>, 
				basic_number>::type>::type
		{
			using return_type_base = typename return_type<decltype(value * other.get_value()), T>::type;
			return basic_number<return_type_base, Tid, Tflags, Tbase>{
					static_cast<return_type_base>(value * other.get_value())
				};
		}
		
		// *this * Tother
		template<typename Tother>
		auto operator*(Tother&& other) const ->
		typename std::enable_if< !(is_equivalent_basic_number<Tother>::value)
			&& (flag_set<ENABLE_GENERAL_MULTIPLICATION>::value
				|| (std::is_floating_point<typename std::remove_reference<Tother>::type>::value 
					&& flag_set<ENABLE_FLOAT_MULTIPLICATION_O_>::value)
				|| (std::is_integral<typename std::remove_reference<Tother>::type>::value 
					&& flag_set<ENABLE_INTEGER_MULTIPLICATION>::value)
				|| (flag_set<ENABLE_BASE_MULTIPLICATION_O_>::value
					&& std::is_floating_point<typename std::remove_reference<Tother>::type>::value
					&& std::is_floating_point<T>::value)
			), typename return_type< 
				basic_number<decltype(value * other), Tid, Tflags, Tbase>, 
				basic_number>::type >::type
		{
			using return_type_base = typename return_type<decltype(value * other), T>::type;
			return basic_number<return_type_base, Tid, Tflags, Tbase>{
					static_cast<return_type_base>(value * other)
				};
		}
		
		// *this *= basic_number
		template<typename Tother>
		typename std::enable_if< is_equivalent_basic_number<Tother>::value
			&& flag_unset<DISABLE_MUTABILITY>::value
			&& flag_set<ENABLE_SPECIFIC_MULTIPLICATION>::value, basic_number&>::type
		operator*=(Tother&& other){
			value *= other.get_value();
			return *this;
		}
		
		// *this *= Tother
		template<typename Tother>
		typename std::enable_if< !is_equivalent_basic_number<Tother>::value
			&& flag_unset<DISABLE_MUTABILITY>::value
			&& (flag_set<ENABLE_GENERAL_MULTIPLICATION>::value
				|| (std::is_floating_point<typename std::remove_reference<Tother>::type>::value 
					&& flag_set<ENABLE_FLOAT_MULTIPLICATION_O_>::value)
				|| (std::is_integral<typename std::remove_reference<Tother>::type>::value 
					&& flag_set<ENABLE_INTEGER_MULTIPLICATION>::value)
				|| (flag_set<ENABLE_BASE_MULTIPLICATION_O_>::value
					&& std::is_floating_point<typename std::remove_reference<Tother>::type>::value
					&& std::is_floating_point<T>::value)
			), basic_number&>::type
		operator*=(Tother&& other){
			value *= other;
			return *this;
		}
		
		
		// *this / basic_number
		template<typename Tother>
		auto operator/(Tother&& other) const ->
		typename std::enable_if< is_equivalent_basic_number<Tother>::value
			&& flag_set<ENABLE_SPECIFIC_DIVISION>::value,
			typename return_type<
				basic_number<decltype(value / other.get_value()), Tid, Tflags, Tbase>, 
				basic_number>::type>::type
		{
			using return_type_base = typename return_type<decltype(value / other.get_value()), T>::type;
			return basic_number<return_type_base, Tid, Tflags, Tbase>{
					static_cast<return_type_base>(value / other.get_value())
				};
		}
		
		// *this / Tother
		template<typename Tother>
		auto operator/(Tother&& other) const ->
		typename std::enable_if< !(is_equivalent_basic_number<Tother>::value)
			&& (flag_set<ENABLE_GENERAL_DIVISION>::value
				|| (std::is_floating_point<typename std::remove_reference<Tother>::type>::value 
					&& flag_set<ENABLE_FLOAT_DIVISION_O_>::value)
				|| (std::is_integral<typename std::remove_reference<Tother>::type>::value 
					&& flag_set<ENABLE_INTEGER_DIVISION>::value)
				|| (flag_set<ENABLE_BASE_DIVISION_O_>::value
					&& std::is_floating_point<typename std::remove_reference<Tother>::type>::value
					&& std::is_floating_point<T>::value)
			), typename return_type< 
				basic_number<decltype(value / other), Tid, Tflags, Tbase>, 
				basic_number>::type >::type
		{
			using return_type_base = typename return_type<decltype(value / other), T>::type;
			return basic_number<return_type_base, Tid, Tflags, Tbase>{
					static_cast<return_type_base>(value / other)
				};
		}
		
		// *this /= basic_number
		template<typename Tother>
		typename std::enable_if< is_equivalent_basic_number<Tother>::value
			&& flag_unset<DISABLE_MUTABILITY>::value
			&& flag_set<ENABLE_SPECIFIC_DIVISION>::value, basic_number&>::type
		operator/=(Tother&& other){
			value /= other.get_value();
			return *this;
		}
		
		// *this /= Tother
		template<typename Tother>
		typename std::enable_if< !is_equivalent_basic_number<Tother>::value
			&& !(Tflags & DISABLE_MUTABILITY)
			&& (flag_set<ENABLE_GENERAL_DIVISION>::value
				|| (std::is_floating_point<typename std::remove_reference<Tother>::type>::value 
					&& flag_set<ENABLE_FLOAT_DIVISION_O_>::value)
				|| (std::is_integral<typename std::remove_reference<Tother>::type>::value 
					&& flag_set<ENABLE_INTEGER_DIVISION>::value)
				|| (flag_set<ENABLE_BASE_DIVISION_O_>::value
					&& std::is_floating_point<typename std::remove_reference<Tother>::type>::value
					&& std::is_floating_point<T>::value)
			), basic_number&>::type
		operator/=(Tother&& other){
			value /= other;
			return *this;
		}
		
		// *this % basic_number
		template<typename Tother>
		typename std::enable_if<is_equivalent_basic_number<Tother>::value
			&& std::is_integral<T>::value
			&& flag_set<ENABLE_SPECIFIC_MODULO>::value, basic_number>::type
		operator%(Tother&& other) const{
			return basic_number(value % other.get_value());
		}
		
		// *this %= basic_number
		template<typename Tother>
		typename std::enable_if<is_equivalent_basic_number<Tother>::value
			&& flag_unset<DISABLE_MUTABILITY>::value
			&& std::is_integral<T>::value
			&& flag_set<ENABLE_SPECIFIC_MODULO>::value, basic_number&>::type
		operator%=(Tother&& other){
			value %= other.get_value();
			return *this;
		}
		
		// *this % Tother
		template<typename Tother>
		typename std::enable_if<!(is_equivalent_basic_number<Tother>::value)
			&& std::is_integral<T>::value
			&& flag_set<ENABLE_MODULO_O_>::value, basic_number>::type
		operator%(Tother&& other) const{
			return basic_number(value % other);
		}
		
		// *this %= Tother
		template<typename Tother>
		typename std::enable_if<!(is_equivalent_basic_number<Tother>::value)
			&& flag_unset<DISABLE_MUTABILITY>::value
			&& std::is_integral<T>::value
			&& flag_set<ENABLE_MODULO_O_>::value, basic_number>::type
		operator%=(Tother&& other){
			value %= other;
			return *this;
		}
		
		///////////////////////////////////////////////
		
		/**
		 * @brief Provides reading access to the underlying value.
		 * @return the value of the underlying variable.
		 */
		T get_value() const{ 
			return value; 
		}
		
	protected:
		
		/**
		 * @brief Sets the value of 'this'. 
		 * @note This method is mainly provided for use by Tbase.
		 * @param value the new value
		 */
		void set_value(T value){
			this->value = value;
		}
};

// these templates check whether a type is an instance of basic_number:
template <typename T>
struct is_basic_number{
	enum {
		value = false
	};
};
template <typename T, class Tid, flag_t Tflags, template<typename, class> class Tbase>
struct is_basic_number<basic_number<T, Tid, Tflags, Tbase>>{
	enum {
		value = true
	};
};

template<typename Tother, class T, class Tid, flag_t Tflags, template<typename, class> class Tbase>
auto operator+(const Tother& other, const basic_number<T, Tid, Tflags, Tbase>& value)
	-> typename std::enable_if<!is_basic_number<Tother>::value, decltype(value.operator+(other))>::type {
	return {value+other};
}

template<typename Tother, class T, class Tid, flag_t Tflags, template<typename, class> class Tbase>
auto operator*(const Tother& factor, const basic_number<T, Tid, Tflags, Tbase>& value)
	-> typename std::enable_if<!is_basic_number<Tother>::value, decltype(value.operator*(factor))>::type {
	return {value*factor};
}

} //namespace type_builder

#endif
