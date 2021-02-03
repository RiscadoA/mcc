#pragma once

#include <mcc/data/loader.hpp>

namespace mcc::data {
    template <typename T>
    class Handle;
    template <>
    class Handle<void>;

    template <typename T>
    class Handle {
    public:
        inline Handle();
        inline Handle(int id, typename T::Loader* loader);
        inline Handle(const Handle<T>& rhs);
        inline Handle(const Handle<void>& rhs);
        inline Handle(Handle&& rhs);
        inline Handle& operator=(const Handle& rhs);
        inline ~Handle();
        inline const T* operator->();

        inline bool is_ready();
        inline Handle& wait();

    private:
        friend Handle<void>;

        typename T::Loader* loader;
        int id;
    };

    template <>
    class Handle<void> {
    public:
        inline Handle();
        inline Handle(int id, Loader* loader);
        template <typename T>
        inline Handle(const Handle<T>& rhs);
        inline Handle(const Handle& rhs);
        inline Handle(Handle&& rhs);
        template <typename T>
        inline Handle& operator=(const Handle<T>& rhs);
        inline ~Handle();

        inline bool is_ready();
        inline Handle<void>& wait();

    private:
        template <typename T>
        friend class Handle;

        Loader* loader;
        int id;
    };

    template <typename T>
    inline Handle<T>::Handle() {
        this->loader = nullptr;
    }

    template <typename T>
    inline Handle<T>::Handle(int id, typename T::Loader* loader) {
        this->id = id;
        this->loader = loader;
        if (this->loader != nullptr) {
            this->loader->inc_ref(this->id);
        }
    }

    template <typename T>
    inline Handle<T>::Handle(const Handle<T>& rhs) {
        this->id = rhs.id;
        this->loader = rhs.loader;
        if (this->loader != nullptr) {
            this->loader->inc_ref(this->id);
        }
    }

    template <typename T>
    inline Handle<T>::Handle(const Handle<void>& rhs) {
        this->id = rhs.id;
        this->loader = (typename T::Loader*)rhs.loader;
        if (this->loader != nullptr) {
            this->loader->inc_ref(this->id);
        }
    }

    template <typename T>
    inline Handle<T>::Handle(Handle&& rhs) {
        this->id = rhs.id;
        this->loader = rhs.loader;
        rhs.loader = nullptr;
    }

    template <typename T>
    inline Handle<T>& Handle<T>::operator=(const Handle& rhs) {
        if (this->loader != nullptr) {
            this->loader->dec_ref(this->id);
        }
        this->id = rhs.id;
        this->loader = rhs.loader;
        if (this->loader != nullptr) {
            this->loader->inc_ref(this->id);
        }
        return *this;
    }

    template <typename T>
    inline Handle<T>::~Handle() {
        if (this->loader != nullptr) {
            this->loader->dec_ref(this->id);
        }
    }
    
    template<typename T>
    inline const T* Handle<T>::operator->() {
        this->wait();
        return this->loader->get(this->id);
    }

    template<typename T>
    inline bool Handle<T>::is_ready() {
        return this->loader->is_ready(this->id);
    }

    template<typename T>
    inline Handle<T>& Handle<T>::wait() {
        this->loader->wait(this->id);
        return *this;
    }

    inline Handle<void>::Handle() {
        this->loader = nullptr;
    }

    inline Handle<void>::Handle(int id, Loader* loader) {
        this->id = id;
        this->loader = loader;
        if (this->loader != nullptr) {
            this->loader->inc_ref(this->id);
        }
    }

    template <typename T>
    inline Handle<void>::Handle(const Handle<T>& rhs) {
        this->id = rhs.id;
        this->loader = rhs.loader;
        if (this->loader != nullptr) {
            this->loader->inc_ref(this->id);
        }
    }

    inline Handle<void>::Handle(const Handle& rhs) {
        this->id = rhs.id;
        this->loader = rhs.loader;
        if (this->loader != nullptr) {
            this->loader->inc_ref(this->id);
        }
    }

    inline Handle<void>::Handle(Handle&& rhs) {
        this->id = rhs.id;
        this->loader = rhs.loader;
        rhs.loader = nullptr;
    }

    template <typename T>
    inline Handle<void>& Handle<void>::operator=(const Handle<T>& rhs) {
        if (this->loader != nullptr) {
            this->loader->dec_ref(this->id);
        }
        this->id = rhs.id;
        this->loader = rhs.loader;
        if (this->loader != nullptr) {
            this->loader->inc_ref(this->id);
        }
        return *this;
    }

    inline Handle<void>::~Handle() {
        if (this->loader != nullptr) {
            this->loader->dec_ref(this->id);
        }
    }

    inline bool Handle<void>::is_ready() {
        return this->loader->is_ready(this->id);
    }

    inline Handle<void>& Handle<void>::wait() {
        this->loader->wait(this->id);
        return *this;
    }
}