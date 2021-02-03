#pragma once

#include <iostream>
#include <utility>

namespace mcc {
	/*
		Abstract result type.
		The type T contains the expected succesful return value.
		The type E is the type of the error returned on failure.
		The type E default constructor should return an error code signiying that there was no error.
		If the type E is an enum, the code corresponding to no error should be 0 (enums are zero-initialized).
		Both type T and E must be either copyable or movable.
		The type E must implement the std::ostream << operator.
	*/
	template <typename T, typename E>
	class Result {
	public:
		inline Result(Result&& rhs) {
			this->is_err = rhs.is_err;
			if (this->is_err)
				new (&this->err) E(std::move(rhs.err));
			else
				new (&this->val) T(std::move(rhs.val));
		}

		inline ~Result() {
			if (this->is_err)
				this->err.~E();
			else
				this->val.~T();
		}

		/*
			Creates a success result.
		*/
		static inline Result<T, E> success(T&& value) {
			return Result(std::move(value), true);
		}

		/*
			Creates an error result.
		*/
		static inline Result<T, E> error(E&& error) {
			return Result(std::move(error), nullptr);
		}

		/*
			Checks if the result contains an error.
		*/
		inline bool is_error() const {
			return this->is_err;
		}

		/*
			Gets the value of the result.
			If there was an error, std::abort() is called and debug info about the error is printed to std::cerr.
		*/
		inline T unwrap() & {
			if (this->is_error()) {
				std::cerr << "Failed to unwrap Result<" << typeid(T).name() << ", " << typeid(E).name() << ">, error:" << std::endl;
				std::cerr << this->err << std::endl;
				std::abort();
			}
 			return this->val;
		}

		/*
			Gets the value of the result.
			If there was an error, std::abort() is called and debug info about the error is printed to std::cerr.
		*/
		inline T&& unwrap() && {
			if (this->is_error()) {
				std::cerr << "Failed to unwrap Result<" << typeid(T).name() << ", " << typeid(E).name() << ">, error:" << std::endl;
				std::cerr << this->err << std::endl;
				std::abort();
			}
			return std::move(this->val);
		}

		/*
			Gets the error value of the result.
			If there was no error, E() is returned.
		*/
		inline E get_error() const {
			if (!this->is_err)
				return E();
			return this->err;
		}

	private:
		inline Result(T&& val, bool) : val(std::move(val)), is_err(false) {}
		inline Result(E&& err, void*) : err(std::move(err)), is_err(true) {}
		
		bool is_err;
		union {
			T val;
			E err;
		};	
	};

	template <typename T, typename E>
	class Result<T&, E> {
	public:
		inline Result(Result&& rhs) {
			this->is_err = rhs.is_err;
			if (this->is_err)
				new (&this->err) E(std::move(rhs.err));
			else
				this->val = rhs.val;
		}

		inline ~Result() {
			if (this->is_err)
				this->err.~E();
		}

		static inline Result<T&, E> success(T& value) {
			return Result(std::move(value), true);
		}

		static inline Result<T&, E> error(E&& error) {
			return Result(std::move(error), nullptr);
		}

		inline bool is_error() const {
			return this->is_err;
		}

		/*
			Gets the value of the result.
			If there was an error, std::abort() is called and debug info about the error is printed to std::cerr.
		*/
		inline T& unwrap() {
			if (this->is_error()) {
				std::cerr << "Failed to unwrap Result<" << typeid(T).name() << ", " << typeid(E).name() << ">, error:" << std::endl;
				std::cerr << this->err << std::endl;
				std::abort();
			}
			return *this->val;
		}

		/*
			Gets the error value of the result.
			If there was no error, E() is returned.
		*/
		inline E get_error() const {
			if (!this->error)
				return E();
			return this->err;
		}

	private:
		inline Result(T& val, bool) : val(&val), is_err(false) {}
		inline Result(E&& err, void*) : err(std::move(err)), is_err(true) {}

		bool is_err;
		union {
			T* val;
			E err;
		};
	};

	template <typename E>
	class Result<void, E> {
	public:
		inline Result(Result&& rhs) {
			this->is_err = rhs.is_err;
			if (this->is_err)
				new (&this->err) E(std::move(rhs.err));
		}

		inline ~Result() {
			if (this->is_err)
				this->err.~E();
		}

		static inline Result<void, E> success() {
			return Result();
		}

		static inline Result<void, E> error(E&& error) {
			return Result(std::move(error));
		}

		inline bool is_error() const {
			return this->is_err;
		}

		/*
			If there was an error, std::abort() is called and debug info about the error is printed to std::cerr.
		*/
		inline void unwrap() {
			if (this->is_error()) {
				std::cerr << "Failed to unwrap Result<void, " << typeid(E).name() << ">, error:" << std::endl;
				std::cerr << this->err << std::endl;
				std::abort();
			}
		}

		/*
			Gets the error value of the result.
			If there was no error, E() is returned.
		*/
		inline E get_error() const {
			if (!this->is_err)
				return E();
			return this->err;
		}

	private:
		inline Result() : is_err(false) {}
		inline Result(E&& err) : err(std::move(err)), is_err(true) {}

		bool is_err;
		E err;
	};
}