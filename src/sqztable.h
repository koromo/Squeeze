#ifndef SQUEEZE_SQZTABLE_H
#define SQUEEZE_SQZTABLE_H

#include "sqzclosure.h"
#include "sqztableimpl.h"
#include "sqzstackop.h"
#include "sqzdef.h"
#include "sqzutil.h"
#include <squirrel.h>
#include <type_traits>

namespace squeeze
{
    template <class Class> class HClass;

    /** The Table object handle */
    class HTable : public HTableImpl
    {
    public:
        /** Construct */
        HTable() = default;

        /** Create a table object */
        explicit HTable(HVM vm)
        {
            vm_ = vm;
            const auto top = sq_gettop(vm_);
            sq_newtable(vm_);
            sq_getstackobj(vm_, -1, &obj_);
            sq_addref(vm_, &obj_);
            sq_settop(vm_, top);
        }

        /** Create with copy the object handle */
        HTable(HVM vm, HSQOBJECT obj)
        {
            vm_ = vm;
            obj_ = obj;
            sq_addref(vm_, &obj_);
        }

        /** Create a clone table */
        HTable clone()
        {
            HTable clo;
            clo.HTableImpl::clone(this);
            return clo;
        }

        /** Add a new slot as a variable. */
        template <class T>
        HTable& var(const string_t& key, const T& val)
        {
            newSlot(key, val, false);
            return *this;
        }

        /** Add a new slot as a table. */
        HTable& table(const string_t& key, HTable table)
        {
            newSlot(key, table, false);
            return *this;
        }

        /** Add a new slot as a class. */
        template <class Class>
        HTable& clazz(const string_t& key, HClass<Class> c);

        /** Add a new slot as a function. */
        template <class F>
        HTable& fun(const string_t& key, const F& f)
        {
            newClosure(key, Closure::fun<F>, false, UserData(&f, sizeof(F)));
            return *this;
        }

        /** Call a function mapped by 'key'. */
        template <class Return, class... Args>
        Return call(const string_t& key, HTable env,  Args&&... args)
        {
            return HTableImpl::call<Return>(key, env, std::forward<Args>(args)...);
        }
    };
}

#endif