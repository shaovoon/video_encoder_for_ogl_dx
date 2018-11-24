/********************************/
/* SDLLib Emscripten Framework  */
/* Copyright © 2016             */
/* Author: Wong Shao Voon       */
/********************************/

#pragma once

#include <string>

namespace Library
{
    class RTTI
    {
    public:
		virtual const size_t& TypeIdInstance() const = 0;
        
		virtual RTTI* QueryInterface(const size_t id) const
        {
            return nullptr;
        }

		virtual bool Is(const size_t id) const
        {
            return false;
        }

        virtual bool Is(const std::string& name) const
        {
            return false;
        }

        template <typename T>
        T* As() const
        {
            if (Is(T::TypeIdClass()))
            {
                return (T*)this;
            }

            return nullptr;
        }
    };

    #define RTTI_DECLARATIONS(Type, ParentType)                                                              \
        public:                                                                                              \
            static std::string TypeName() { return std::string(#Type); }                                     \
            virtual const size_t& TypeIdInstance() const  override { return Type::TypeIdClass(); }           \
            static  const size_t& TypeIdClass() { return sRunTimeTypeId; }                                   \
            virtual Library::RTTI* QueryInterface( const size_t id ) const override                          \
            {                                                                                                \
                if (id == sRunTimeTypeId)                                                                    \
                    { return (RTTI*)this; }                                                                  \
                else                                                                                         \
                    { return ParentType::QueryInterface(id); }                                               \
            }                                                                                                \
            virtual bool Is(const size_t id) const override                                            \
            {                                                                                                \
                if (id == sRunTimeTypeId)                                                                    \
                    { return true; }                                                                         \
                else                                                                                         \
                    { return ParentType::Is(id); }                                                           \
            }                                                                                                \
            virtual bool Is(const std::string& name) const override                                          \
            {                                                                                                \
                if (name == TypeName())                                                                      \
                    { return true; }                                                                         \
                else                                                                                         \
                    { return ParentType::Is(name); }                                                         \
            }                                                                                                \
       private:                                                                                              \
            static size_t sRunTimeTypeId;

	#define RTTI_DEFINITIONS(Type) size_t Type::sRunTimeTypeId = (size_t)& Type::sRunTimeTypeId;
}