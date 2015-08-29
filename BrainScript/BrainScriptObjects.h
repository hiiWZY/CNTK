// BrainScriptObjects.h -- objects that the config parser operates on

#pragma once

#include <memory>       // for shared_ptr<>
#include <functional>   // for function<>

namespace Microsoft { namespace MSR { namespace CNTK { namespace BS { // or BS::Config? or MSR::BS?

    using namespace std;

    // TODO: comment this
    typedef shared_ptr<struct IConfigRecord> IConfigRecordPtr;

    // -----------------------------------------------------------------------
    // Object -- common base class for objects that can be used in config files
    // -----------------------------------------------------------------------

    // All values that can be used in config files
    //  - are heap objects
    //     - primitives are wrapped
    //     - object pointers are ref-counted shared_ptr, wrapped in ConfigValuePtr (see BrainScriptEvaluator.h)
    //  - derive from Object (outside classes get wrapped)
    //
    // This code supports three kinds of value types:
    //  - self-defined classes -> derive from Object, e.g. Expression
    //  - classes defined outside -> wrap in a BoxOf object, e.g. String = BoxOf<wstring>
    //  - C++ primitives like 'double' -> wrap in a Wrapper first then in a BoxOf, e.g. Number = BoxOf<Wrapped<double>>

    struct Object { virtual ~Object() { } };

    // indicates that the object has a name should be set from the expression path

    struct HasName { virtual void SetName(const wstring & name) = 0; };

    // -----------------------------------------------------------------------
    // Wrapped<T> -- wraps non-class primitive C++ type into a class, like 'double'.
    // (It can also be used for class types, but better use BoxOf<> below directly.)
    // -----------------------------------------------------------------------

    template<typename T> class Wrapped
    {
        T value;    // meant to be a primitive type
    public:
        operator const T&() const { return value; }
        operator T&() { return value; }
        Wrapped(T value) : value(value) { }
        T & operator=(const T & newValue) { value = newValue; }
    };
    typedef Wrapped<double> Double;
    typedef Wrapped<bool> Bool;

    // -----------------------------------------------------------------------
    // BoxOf<T> -- wraps a pre-defined type, e.g. std::wstring, to derive from Object.
    // BoxOf<T> can dynamic_cast to T (e.g. BoxOf<wstring> is a wstring).
    // -----------------------------------------------------------------------

    template<class C>
    class BoxOf : public Object, public C
    {
    public:
        BoxOf(const C & val) : C(val) { }
        BoxOf(){}
    };

    // -----------------------------------------------------------------------
    // String -- a string in config files
    // Can cast to wstring (done in a way that ConfigValuePtr can also cast to wstring).
    // -----------------------------------------------------------------------

    typedef BoxOf<wstring> String;

    // -----------------------------------------------------------------------
    // HasToString -- trait to indicate an object can print their content
    // Derive from HasToString() and implement ToString() method.
    // FormatConfigValue() will then return ToString().
    // -----------------------------------------------------------------------

    struct HasToString { virtual wstring ToString() const = 0; };

    // some useful string helpers
    wstring IndentString(wstring s, size_t indent);
    wstring NestString(wstring s, wchar_t open, bool newline, wchar_t close);
    template<class C> static wstring TypeId() { return msra::strfun::utf16(typeid(C).name()); }

    // TODO: where does this belong? We need to define the minimal interface to runtime types. (They will still need the type casts eventually.)
    // helper for configurableRuntimeTypes initializer below
    // This returns a ConfigurableRuntimeType info structure that consists of
    //  - a lambda that is a constructor for a given runtime type and
    //  - a bool saying whether T derives from IConfigRecord
    struct ConfigurableRuntimeType
    {
        bool isConfigRecord;        // exposes IConfigRecord  --in this case the expression name is computed differently, namely relative to this item
        function<shared_ptr<Object>(const IConfigRecordPtr)> construct; // lambda to construct an object of this class
    };

}}}} // end namespaces
