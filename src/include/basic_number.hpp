#ifndef BASIC_NUMBER_HPP
#define BASIC_NUMBER_HPP

#include <utility>
#include <cstdint>
#include <ostream>
#include <istream>
#include <string>
#include <type_traits>

#include "policy_types.hpp"

namespace type_builder{

enum: uint64_t{
	ENABLE_GENERAL_CONSTRUCTION = 1,
	ENABLE_DEFAULT_CONSTRUCTION = 1 << 1,
	ENABLE_LATE_ASSIGNEMENT = 1 << 2,
	ENABLE_SPECIFIC_EQUALITY_CHECK = 1 << 3,
	ENABLE_SPECIFIC_ORDERING = ENABLE_SPECIFIC_EQUALITY_CHECK | 1 << 4,
	ENABLE_EQUALITY_CHECK = ENABLE_SPECIFIC_EQUALITY_CHECK | 1 << 5,
	ENABLE_ORDERING = ENABLE_SPECIFIC_ORDERING | ENABLE_EQUALITY_CHECK | 1 << 6,
	
	ENABLE_INC_DEC = 1 << 7,
	
	ENABLE_SPECIFIC_PLUS_MINUS = 1 << 8,
	ENABLE_SPECIFIC_MULTIPLICATION = 1 << 9,
	ENABLE_SPECIFIC_DIVISION = 1 << 9,
	
	ENABLE_INTEGER_MULTIPLICATION = 1 << 10,
	ENABLE_INTEGER_DIVISION = 1 << 11,
	
	ENABLE_FLOAT_MULTIPLICATION = ENABLE_INTEGER_MULTIPLICATION | 1 << 12,
	ENABLE_FLOAT_DIVISION = ENABLE_INTEGER_DIVISION | 1 << 13,
	
	ENABLE_INTEGER_MULT_DIV = ENABLE_INTEGER_MULTIPLICATION | ENABLE_INTEGER_DIVISION,
	ENABLE_FLOAT_MULT_DIV = ENABLE_FLOAT_MULTIPLICATION | ENABLE_FLOAT_DIVISION,
	
	ENABLE_GENERAL_PLUS_MINUS = 1 << 20,
	ENABLE_GENERAL_MULTIPLICATION = 1 << 21,
	ENABLE_GENERAL_DIVISION = 1 << 22,
	ENABLE_GENERAL_MULT_DIV = ENABLE_GENERAL_MULTIPLICATION | ENABLE_GENERAL_DIVISION,
	
	ENABLE_SPECIFIC_MODULO = 1 << 26,
	ENABLE_MODULO = ENABLE_SPECIFIC_MODULO | 1 << 27,
	
	DISABLE_CONSTRUCTION = 1 << 30,
	DISABLE_MUTABILITY = 1L << 31,
	
	DEFAULT_SETTINGS = ENABLE_SPECIFIC_ORDERING | ENABLE_INC_DEC
		| ENABLE_SPECIFIC_PLUS_MINUS
		| ENABLE_INTEGER_MULTIPLICATION | ENABLE_INTEGER_DIVISION,
	
	ENABLE_ALL_SPECIFIC_MATH = ENABLE_INC_DEC | ENABLE_SPECIFIC_PLUS_MINUS
		| ENABLE_SPECIFIC_MULTIPLICATION | ENABLE_SPECIFIC_DIVISION
		| ENABLE_SPECIFIC_MODULO,
	
	ENABLE_ALL_MATH = ENABLE_ALL_SPECIFIC_MATH | ENABLE_GENERAL_PLUS_MINUS 
		| ENABLE_GENERAL_MULTIPLICATION | ENABLE_GENERAL_DIVISION
		| ENABLE_FLOAT_MULT_DIV | ENABLE_MODULO,
	
	ENABLE_ALL = ENABLE_GENERAL_CONSTRUCTION | ENABLE_DEFAULT_CONSTRUCTION 
		| ENABLE_LATE_ASSIGNEMENT | ENABLE_ORDERING 
		| ENABLE_ALL_MATH
};

template<
	typename T,
	int Tid,
	uint64_t Tflags = DEFAULT_SETTINGS,
	class Tbase = empty_base
>
class basic_number: public Tbase {
	
	//this template is used to check, whether a type is equal enough
	//to the instanciation of the class-template:
	template<typename Targ> struct is_this{ 
		enum{
			value = ( std::is_same<const basic_number&, const Targ&>::value 
				|| std::is_same<basic_number&, Targ&>::value)
		};
	};
	
	public:
		
		// constructors:
		
		basic_number() : value(0) {
			static_assert( !(Tflags & DISABLE_CONSTRUCTION), 
					"construction of this type is disabled");
			static_assert( Tflags & ENABLE_DEFAULT_CONSTRUCTION,
					"default constructor not enabled for this type");
		}
		
		explicit basic_number(T value): value(value){
			static_assert( !(Tflags & DISABLE_CONSTRUCTION), 
					"construction of this type is disabled");
		}
		
		basic_number(const basic_number& other): value(other.value){
			static_assert( !(Tflags & DISABLE_CONSTRUCTION), 
					"construction of this type is disabled");
		}
		
		basic_number(basic_number&& other): value(std::move(other.value)){
			static_assert( !(Tflags & DISABLE_CONSTRUCTION), 
					"construction of this type is disabled");
		}
		
		template<typename Tother>
		explicit basic_number(const Tother& value): value(value){ 
			static_assert( !(Tflags & DISABLE_CONSTRUCTION), 
					"construction of this type is disabled");
			static_assert(Tflags & ENABLE_GENERAL_CONSTRUCTION, 
					"invalid construction of number type"); 
		}
		
		// methods:
		
		// *this = basic_number
		template<typename Tother> 
		typename std::enable_if<
			is_this<Tother>::value
			&& (!(Tflags & DISABLE_MUTABILITY)),
		basic_number&>::type
		operator=(Tother&& other){
			// we should check for self-assignement:
			if(this != &other){
				this->value = std::forward<Tother>(other).value;
			}
			return *this;
		}
		
		// *this = T
		template<typename Tother> 
		typename std::enable_if<
			!is_this<Tother>::value 
			&& (!(Tflags & DISABLE_MUTABILITY))
			&& (Tflags & ENABLE_LATE_ASSIGNEMENT),
		basic_number&>::type
		operator=(Tother&& other){
			this->value = std::forward<Tother>(other.value);
			return *this;
		}
		
		// *this = Tother
		template<typename Tother>
		typename std::enable_if< 
			!is_this<Tother>::value
			&& !(std::is_same<T&, Tother&>::value)
			&& (Tflags & ENABLE_LATE_ASSIGNEMENT)
			&& (Tflags & ENABLE_GENERAL_CONSTRUCTION),
		basic_number&>::type
		operator=(Tother&& value){
			this->value = std::forward<Tother>(value);
			return *this;
		}
		
		// *this == basic_number
		template <typename Tother>
		typename std::enable_if< is_this<Tother>::value
			&& (Tflags & ENABLE_EQUALITY_CHECK), bool>::type
		operator==(Tother&& other) const{
			return value == other.value;
		}
		
		// *this == Tother
		template <typename Tother>
		typename std::enable_if< !(is_this<Tother>::value)
			&& (Tflags & ENABLE_EQUALITY_CHECK), bool>::type
		operator==(Tother&& other) const{
			return value == other;
		}
		
		// *this != basic_number
		template <typename Tother>
		typename std::enable_if< is_this<Tother>::value
			&& (Tflags & ENABLE_EQUALITY_CHECK), bool>::type
		operator!=(Tother&& other) const{
			return value != other.value;
		}
		
		// *this != Tother
		template <typename Tother>
		typename std::enable_if< !(is_this<Tother>::value)
			&& (Tflags & ENABLE_EQUALITY_CHECK), bool>::type
		operator!=(Tother&& other) const{
			return value != other;
		}
		
		// *this < basic_number
		template <typename Tother>
		typename std::enable_if< is_this<Tother>::value
			&& (Tflags & ENABLE_ORDERING), bool>::type
		operator<(Tother&& other) const{
			return value < other.value;
		}
		
		// *this < Tother
		template <typename Tother>
		typename std::enable_if< !(is_this<Tother>::value)
			&& (Tflags & ENABLE_ORDERING), bool>::type
		operator<(Tother&& other) const{
			return value < other;
		}
		
		// *this <= basic_number
		template <typename Tother>
		typename std::enable_if< is_this<Tother>::value
			&& (Tflags & ENABLE_ORDERING), bool>::type
		operator<=(Tother&& other) const{
			return value <= other.value;
		}
		
		// *this <= Tother
		template <typename Tother>
		typename std::enable_if< !(is_this<Tother>::value)
			&& (Tflags & ENABLE_ORDERING), bool>::type
		operator<=(Tother&& other) const{
			return value <= other;
		}
		
		// *this > basic_number
		template <typename Tother>
		typename std::enable_if< is_this<Tother>::value
			&& (Tflags & ENABLE_ORDERING), bool>::type
		operator>(Tother&& other) const{
			return value > other.value;
		}
		
		// *this > Tother
		template <typename Tother>
		typename std::enable_if< !(is_this<Tother>::value)
			&& (Tflags & ENABLE_ORDERING), bool>::type
		operator>(Tother&& other) const{
			return value > other;
		}
		
		
		// *this >= basic_number
		template <typename Tother>
		typename std::enable_if< is_this<Tother>::value
			&& (Tflags & ENABLE_ORDERING), bool>::type
		operator>=(Tother&& other) const{
			return value >= other.value;
		}
		
		// *this >= Tother
		template <typename Tother>
		typename std::enable_if< !(is_this<Tother>::value)
			&& (Tflags & ENABLE_ORDERING), bool>::type
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
		typename std::enable_if< is_this<Tother>::value
			&& (Tflags & ENABLE_SPECIFIC_PLUS_MINUS), basic_number>::type
		operator+(Tother&& other) const{
			return basic_number{static_cast<T>(value + other.value)};
		}
		
		// *this + Tother
		template<typename Tother>
		typename std::enable_if< !(is_this<Tother>::value)
			&& (Tflags & ENABLE_GENERAL_PLUS_MINUS), basic_number>::type
		operator+(Tother&& other) const{
			return basic_number{static_cast<T>(value + other)};
		}
		
		// *this += basic_number
		template<typename Tother>
		typename std::enable_if< is_this<Tother>::value
			&& !(Tflags & DISABLE_MUTABILITY)
			&& (Tflags & ENABLE_SPECIFIC_PLUS_MINUS), basic_number&>::type
		operator+=(Tother&& other){
			value += other.value;
			return *this;
		}
		
		// *this += Tother
		template<typename Tother>
		typename std::enable_if< !is_this<Tother>::value
			&& !(Tflags & DISABLE_MUTABILITY)
			&& (Tflags & ENABLE_GENERAL_PLUS_MINUS), basic_number&>::type
		operator+=(const Tother& other){
			value += other;
			return *this;
		}
		
		// *this - basic_number
		template<typename Tother>
		typename std::enable_if< is_this<Tother>::value
			&& (Tflags & ENABLE_SPECIFIC_PLUS_MINUS), basic_number>::type
		operator-(Tother&& other) const{
			return basic_number{static_cast<T>(value - other.value)};
		}
		
		// *this - Tother
		template<typename Tother>
		typename std::enable_if< !(is_this<Tother>::value)
			&& (Tflags & ENABLE_GENERAL_PLUS_MINUS), basic_number>::type
		operator-(Tother&& other) const{
			return basic_number{static_cast<T>(value - other)};
		}
		
		// *this -= basic_number
		template<typename Tother>
		typename std::enable_if< is_this<Tother>::value
			&& !(Tflags & DISABLE_MUTABILITY)
			&& (Tflags & ENABLE_SPECIFIC_PLUS_MINUS), basic_number&>::type
		operator-=(Tother&& other){
			value -= other.value;
			return *this;
		}
		
		// *this -= Tother
		template<typename Tother>
		typename std::enable_if< !is_this<Tother>::value
			&& !(Tflags & DISABLE_MUTABILITY)
			&& (Tflags & ENABLE_GENERAL_PLUS_MINUS), basic_number&>::type
		operator-=(const Tother& other){
			value -= other;
			return *this;
		}
		
		// *this * basic_number
		template<typename Tother>
		typename std::enable_if< is_this<Tother>::value
			&& (Tflags & ENABLE_SPECIFIC_MULTIPLICATION), basic_number>::type
		operator*(Tother&& other) const{
			return basic_number{static_cast<T>(value * other.value)};
		}
		
		// *this * Tother
		template<typename Tother>
		typename std::enable_if< !(is_this<Tother>::value)
			&& ((Tflags & ENABLE_GENERAL_MULTIPLICATION)
				|| (std::is_floating_point<typename std::remove_reference<Tother>::type>::value 
					&& (Tflags & ENABLE_FLOAT_MULTIPLICATION))
				|| (std::is_integral<typename std::remove_reference<Tother>::type>::value 
					&& (Tflags & ENABLE_INTEGER_MULTIPLICATION))
			), basic_number>::type
		operator*(Tother&& other) const{
			return basic_number{static_cast<T>(value * other)};
		}
		
		// *this *= basic_number
		template<typename Tother>
		typename std::enable_if< is_this<Tother>::value
			&& !(Tflags & DISABLE_MUTABILITY)
			&& (Tflags & ENABLE_SPECIFIC_MULTIPLICATION), basic_number&>::type
		operator*=(Tother&& other){
			value *= other.value;
			return *this;
		}
		
		// *this *= Tother
		template<typename Tother>
		typename std::enable_if< !is_this<Tother>::value
			&& !(Tflags & DISABLE_MUTABILITY)
			&& ((Tflags & ENABLE_GENERAL_MULTIPLICATION)
				|| (std::is_floating_point<typename std::remove_reference<Tother>::type>::value 
					&& Tflags & ENABLE_FLOAT_MULTIPLICATION)
				|| (std::is_integral<typename std::remove_reference<Tother>::type>::value 
					&& Tflags & ENABLE_INTEGER_MULTIPLICATION)
			), basic_number&>::type
		operator*=(Tother&& other){
			value *= other;
			return *this;
		}
		
		
		// *this / basic_number
		template<typename Tother>
		typename std::enable_if< is_this<Tother>::value
			&& (Tflags & ENABLE_SPECIFIC_DIVISION), basic_number>::type
		operator/(Tother&& other) const{
			return basic_number{static_cast<T>(value / other.value)};
		}
		
		// *this / Tother
		template<typename Tother>
		typename std::enable_if< !(is_this<Tother>::value)
			&& ((Tflags & ENABLE_GENERAL_DIVISION)
				|| (std::is_floating_point<typename std::remove_reference<Tother>::type>::value 
					&& (Tflags & ENABLE_FLOAT_DIVISION))
				|| (std::is_integral<typename std::remove_reference<Tother>::type>::value 
					&& (Tflags & ENABLE_INTEGER_DIVISION))
			), basic_number>::type
		operator/(Tother&& other) const{
			return basic_number{static_cast<T>(value / other)};
		}
		
		// *this /= basic_number
		template<typename Tother>
		typename std::enable_if< is_this<Tother>::value
			&& !(Tflags & DISABLE_MUTABILITY)
			&& (Tflags & ENABLE_SPECIFIC_DIVISION), basic_number&>::type
		operator/=(Tother&& other){
			value /= other.value;
			return *this;
		}
		
		// *this /= Tother
		template<typename Tother>
		typename std::enable_if< !is_this<Tother>::value
			&& !(Tflags & DISABLE_MUTABILITY)
			&& ((Tflags & ENABLE_GENERAL_DIVISION)
				|| (std::is_floating_point<typename std::remove_reference<Tother>::type>::value 
					&& Tflags & ENABLE_FLOAT_DIVISION)
				|| (std::is_integral<typename std::remove_reference<Tother>::type>::value 
					&& Tflags & ENABLE_INTEGER_DIVISION)
			), basic_number&>::type
		operator/=(Tother&& other){
			value /= other;
			return *this;
		}
		
		// *this % basic_number
		template<typename Tother>
		typename std::enable_if<is_this<Tother>::value
			&& std::is_integral<T>::value
			&& (Tflags & ENABLE_SPECIFIC_MODULO), basic_number>::type
		operator%(Tother&& other) const{
			return basic_number(value % other.value);
		}
		
		// *this %= basic_number
		template<typename Tother>
		typename std::enable_if<is_this<Tother>::value
			&& !(Tflags & DISABLE_MUTABILITY)
			&& std::is_integral<T>::value
			&& (Tflags & ENABLE_SPECIFIC_MODULO), basic_number&>::type
		operator%=(Tother&& other){
			value %= other.value;
			return *this;
		}
		
		// *this % Tother
		template<typename Tother>
		typename std::enable_if<!(is_this<Tother>::value)
			&& std::is_integral<T>::value
			&& (Tflags & ENABLE_MODULO), basic_number>::type
		operator%(Tother&& other) const{
			return basic_number(value % other);
		}
		
		// *this %= Tother
		template<typename Tother>
		typename std::enable_if<!(is_this<Tother>::value)
			&& !(Tflags & DISABLE_MUTABILITY)
			&& std::is_integral<T>::value
			&& (Tflags & ENABLE_MODULO), basic_number>::type
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
		
		enum: uint64_t{
			FLAGS = Tflags
		};
		
		enum: int {
			TYPE_ID = Tid
		};
		
	private:
		T value;
};

template<typename Tother, class T, int Tid, uint64_t Tflags, class Tbase>
basic_number<T, Tid, Tflags, Tbase> operator+(const Tother& factor, 
		const basic_number<T, Tid, Tflags, Tbase>& value){
	return {value+factor};
}

template<typename Tother, class T, int Tid, uint64_t Tflags, class Tbase>
basic_number<T, Tid, Tflags, Tbase> operator*(const Tother& factor, 
		const basic_number<T, Tid, Tflags, Tbase>& value){
	return {value*factor};
}

template<typename Tchar, typename T, int Tid, uint64_t Tflags, class Tbase>
::std::basic_ostream<Tchar>& operator << (::std::basic_ostream<Tchar>& stream,
		const type_builder::basic_number<T, Tid, Tflags, Tbase>& number){
	stream << Tbase::template format<Tchar>(number.get_value());
	return stream;
}

template<typename Tchar, typename T, int Tid, uint64_t Tflags>
::std::basic_ostream<Tchar>& operator << (::std::basic_ostream<Tchar>& stream,
		const type_builder::basic_number<T, Tid, Tflags, empty_base> number){
	stream << number.get_value();
	return stream;
}

template<typename Tchar, typename T, int Tid, uint64_t Tflags, class Tbase>
::std::basic_istream<Tchar>& operator >> (::std::basic_istream<Tchar>& stream,
		type_builder::basic_number<T, Tid, Tflags, Tbase>& number){
	type_builder::basic_number<T,Tid, Tflags, Tbase>(
			Tbase::read_istream(stream), number);
	return stream;
}

template<typename Tchar, typename T, int Tid, uint64_t Tflags>
::std::basic_istream<Tchar>& operator >> (::std::basic_istream<Tchar>& stream,
		type_builder::basic_number<T, Tid, Tflags, empty_base>& number){
	T tmp;
	stream >> tmp;
	number = type_builder::basic_number<T,Tid, Tflags, empty_base>(tmp);
	return stream;
}

} //namespace type_builder

#endif





