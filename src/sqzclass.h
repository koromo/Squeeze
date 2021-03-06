#ifndef SQUEEZE_SQZCLASS_H
#define SQUEEZE_SQZCLASS_H

#include "sqzclosure.h"
#include "sqztable.h"
#include "sqztableimpl.h"
#include "sqzstackop.h"
#include "sqzdef.h"
#include <squirrel.h>
#include <type_traits>
#include <tuple>

namespace squeeze
{
    /** The Class object handle */
    template <class Class>
    class HClass : public HTableImpl
    {
    private:
        HTable setTable_;
        HTable getTable_;

    public:
        /** Construct */
        HClass() = default;

        /** Create a class object */
        explicit HClass(HVM vm)
            : setTable_(vm)
            , getTable_(vm)
        {
            vm_ = vm;
            const auto top = sq_gettop(vm_);
            sq_newclass(vm_, SQFalse);
            sq_getstackobj(vm_, -1, &obj_);
            sq_addref(vm_, &obj_);
            sq_settop(vm_, top);
            init();
        }

        /** Create a class object with extend 'base' class. */
        template <class U, class = std::enable_if_t<std::is_base_of<U, Class>::value>>
        explicit HClass(HClass<U> base)
            : setTable_(base.setTable_.clone())
            , getTable_(base.getTable_.clone())
        {
            vm_ = base.vm();
            const auto top = sq_gettop(vm_);
            pushValue(vm_, base);
            sq_newclass(vm_, SQTrue);
            sq_getstackobj(vm_, -1, &obj_);
            sq_addref(vm_, &obj_);
            sq_settop(vm_, top);
            init();
        }

        /** Add constructor */
        template <class... Args>
        HClass& ctor()
        {
            newClosure(SQZ_T("constructor"), CtorClosure<Class>::ctor<Args...>, false);
            return *this;
        }

        /** Add a member as a variable */
        template <class T>
        HClass& var(const string_t& name, T&& val, bool isStatic = false)
        {
            newSlot(name, std::forward<T>(val), isStatic);
            return *this;
        }

        /** Add a member as a table */
        HClass& table(const string_t& name, HTable table, bool isStatic = false)
        {
            newSlot(name, table, isStatic);
            return *this;
        }

        /** Add a member as a setter. */
        template <class Setter, class = std::enable_if_t<std::is_same<ReturnType<Setter>, void>::value>>
        HClass& setter(const string_t& name, const Setter& set)
        {
            setTable_.newClosure(name, Closure::memfun<Setter, Class>, false, UserData(&set, sizeof(Setter)));
            return *this;
        }

        /** Add a member as a getter. */
        template <class Getter, class = std::enable_if_t<!std::is_same<ReturnType<Getter>, void>::value>>
        HClass& getter(const string_t& name, const Getter& get)
        {
            getTable_.newClosure(name, Closure::memfun<Getter, Class>, false, UserData(&get, sizeof(Getter)));
            return *this;
        }

        /** Add a member as a property. */
        template <class Getter, class Setter>
        HClass& prop(const string_t& name, const Getter& get, const Setter& set)
        {
            getter(name, get);
            setter(name, set);
            return *this;
        }

        /** Add a member as a non-static function */
        template <class F>
        HClass& fun(const string_t& name, const F& f)
        {
            newClosure(name, Closure::memfun<F, Class>, false, UserData(&f, sizeof(F)));
            return *this;
        }

        /** Add a member as a static function */
        template <class F>
        HClass& staticFun(const string_t& name, const F& f)
        {
            newClosure(name, Closure::fun<F>, true, UserData(&f, sizeof(F)));
            return *this;
        }

    private:
        void init()
        {
            newClosure(SQZ_T("_set"), Closure::opSet, false, setTable_);
            newClosure(SQZ_T("_get"), Closure::opGet, false, getTable_);
        }
    };
}

#endif
