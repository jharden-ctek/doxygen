#ifndef TEMPLATE_H
#define TEMPLATE_H

#include <qcstring.h>
#include <qvaluelist.h>

class FTextStream;

class TemplateListIntf;
class TemplateStructIntf;
class TemplateEngine;

/** @defgroup template_api Template API 
 *
 *  This is the API for a 
 *  <a href="https://docs.djangoproject.com/en/1.6/topics/templates/">Django</a> 
 *  compatible template system written in C++.
 *  It is somewhat inspired by Stephen Kelly's 
 *  <a href="http://www.gitorious.org/grantlee/pages/Home">Grantlee</a>.
 *
 *  A template is simply a text file. 
 *  A template contains \b variables, which get replaced with values when the 
 *  template is evaluated, and \b tags, which control the logic of the template.
 *
 *  Variables look like this: `{{ variable }}`
 *  When the template engine encounters a variable, it evaluates that variable and 
 *  replaces it with the result. Variable names consist of any combination of 
 *  alphanumeric characters and the underscore ("_").
 *  Use a dot (.) to access attributes of a variable.
 *  
 *  One can modify variables for display by using \b filters, for example:
 *  `{{ value|default:"nothing" }}`
 *
 *  Tags look like this: `{% tag %}`. Tags are more complex than variables: 
 *  Some create text in the output, some control flow by performing loops or logic, 
 *  and some load external information into the template to be used by later variables.
 *
 *  To comment-out part of a line in a template, use the comment syntax: 
 *  `{# comment text #}`.
 *
 *  Supported Django tags:
 *  - `for ... empty ... endfor`
 *  - `if ... else ... endif` 
 *  - `block ... endblock` 
 *  - `extends`
 *  - `include`
 *
 *  Supported Django filters:
 *  - `default`
 *  - `length`
 *  - `add`
 *
 *  Extension tags:
 *  - `create` which instantiates a template and writes the result to a file.
 *  The syntax is `{% create 'filename' from 'template' %}`.
 *
 *  @{
 */

/** @brief Variant type which can hold one value of a fixed set of types. */
class TemplateVariant
{
  public:
    /** Signature of the callback function, used for function type variants */
    typedef QCString (*FuncType)(const void *obj, const QValueList<TemplateVariant> &args);

    /** Types of data that can be stored in a TemplateVariant */
    enum Type { None, Bool, Integer, String, Struct, List, Function };

    /** Returns the type of the value stored in the variant */
    Type type() const;

    /** Returns TRUE if the variant holds a valid value, or FALSE otherwise */
    bool isValid() const;

    /** Constructs an invalid variant. */
    TemplateVariant();

    /** Constructs a new variant with a boolean value \a b. */
    explicit TemplateVariant(bool b);

    /** Constructs a new variant with a integer value \a v. */
    TemplateVariant(int v);

    /** Constructs a new variant with a string value \a s. */
    TemplateVariant(const char *s);

    /** Constructs a new variant with a string value \a s. */
    TemplateVariant(const QCString &s);

    /** Constructs a new variant with a struct value \a s. 
     *  @note. Only a pointer to the struct is stored. The caller
     *  is responsible to manage the memory for the struct object.
     */
    TemplateVariant(const TemplateStructIntf *s);

    /** Constructs a new variant with a list value \a l. 
     *  @note. Only a pointer to the struct is stored. The caller
     *  is responsible to manage the memory for the list object.
     */
    TemplateVariant(const TemplateListIntf *l);

    /** Constructs a new variant which represents a function 
     *  @param[in] obj Opaque user defined pointer, which
     *             is passed back when call() is invoked.
     *  @param[in] func Callback function to invoke when
     *             calling call() on this variant.
     */
    TemplateVariant(const void *obj,FuncType func);

    /** Destroys the Variant object */
    ~TemplateVariant();

    /** Constructs a copy of the variant, \a v, 
     *  passed as the argument to this constructor.
     */
    TemplateVariant(const TemplateVariant &v);

    /** Assigns the value of the variant \a v to this variant. */
    TemplateVariant &operator=(const TemplateVariant &v);

    /** Compares this QVariant with v and returns true if they are equal; 
     *  otherwise returns false.
     */
    bool operator==(TemplateVariant &other);

    /** Returns the variant as a string. */
    QCString            toString() const;

    /** Returns the variant as a boolean. */
    bool                toBool() const;

    /** Returns the variant as an integer. */
    int                 toInt() const;

    /** Returns the pointer to list referenced by this variant 
     *  or 0 if this variant does not have list type. 
     */
    const TemplateListIntf   *toList() const;

    /** Returns the pointer to struct referenced by this variant 
     *  or 0 if this variant does not have struct type. 
     */
    const TemplateStructIntf *toStruct() const;

    /** Return the result of apply this function with \a args.
     *  Returns an empty string if the variant type is not a function.
     */
    QCString call(const QValueList<TemplateVariant> &args);

    /** Sets whether or not the value of the Variant should be 
     *  escaped or written as-is (raw).
     *  @param[in] b TRUE means write as-is, FALSE means apply escaping.
     */
    void setRaw(bool b);

    /** Returns whether or not the value of the Value is raw.
     *  @see setRaw()
     */
    bool raw() const;
    
  private:
    class Private;
    Private *p;
};

//------------------------------------------------------------------------

/** @brief Abstract read-only interface for a context value of type list. 
 *  @note The values of the list are TemplateVariants.
 */
class TemplateListIntf
{
  public:
    /** @brief Abstract interface for a iterator of a list. */
    class ConstIterator
    {
      public:
        /** Destructor for the iterator */
        virtual ~ConstIterator() {}
        /** Moves iterator to the first element in the list */
        virtual void toFirst() = 0;
        /** Moves iterator to the last element in the list */
        virtual void toLast() = 0;
        /** Moves iterator to the next element in the list */
        virtual void toNext() = 0;
        /** Moves iterator to the previous element in the list */
        virtual void toPrev() = 0;
        /* Returns TRUE if the iterator points to a valid element
         * in the list, or FALSE otherwise.
         * If TRUE is returned, the value pointed to be the 
         * iterator is assigned to \a v.
         */
        virtual bool current(TemplateVariant &v) const = 0;
    };

    /** Destroys the list */
    virtual ~TemplateListIntf() {}

    /** Returns the number of elements in the list */
    virtual int count() const = 0;

    /** Returns the element at index position \a index. */
    virtual TemplateVariant  at(int index) const = 0;

    /** Creates a new iterator for this list. 
     *  @note the user should call delete on the returned pointer.
     */
    virtual TemplateListIntf::ConstIterator *createIterator() const = 0;
};

/** @brief Default implementation of a context value of type list. */
class TemplateList : public TemplateListIntf
{
  public:
    /** Creates a list */
    TemplateList();
    /** Destroys the list */
   ~TemplateList();

    // TemplateListIntf methods
    virtual int  count() const;
    virtual TemplateVariant at(int index) const;
    virtual TemplateListIntf::ConstIterator *createIterator() const;
    
    /** Appends element \a v to the end of the list */
    virtual void append(const TemplateVariant &v);

  private:
    friend class TemplateListConstIterator;
    class Private;
    Private *p;
};

//------------------------------------------------------------------------

/** @brief Abstract interface for a context value of type struct. */
class TemplateStructIntf
{
  public:
    /** Destroys the struct */
    virtual ~TemplateStructIntf() {}

    /** Gets the value for a field name.
     *  @param[in] name The name of the field.
     */
    virtual TemplateVariant get(const char *name) const = 0;
};


/** @brief Default implementation of a context value of type struct. */
class TemplateStruct : public TemplateStructIntf
{
  public:
    /** Creates a struct */
    TemplateStruct();
    /** Destroys the struct */
    virtual ~TemplateStruct();

    // TemplateStructIntf methods
    virtual TemplateVariant get(const char *name) const;

    /** Sets the value the field of a struct
     *  @param[in] name The name of the field.
     *  @param[in] v The value to set.
     */
    virtual void set(const char *name,const TemplateVariant &v);

  private:
    class Private;
    Private *p;
};

//------------------------------------------------------------------------

/** @brief Interface used to escape characters in a string */
class TemplateEscapeIntf
{
  public:
    /** Returns the \a input after escaping certain characters */
    virtual QCString escape(const QCString &input) = 0;
};

//------------------------------------------------------------------------

/** @brief Abstract interface for a template context. 
 *  
 *  A Context consists of a stack of dictionaries.
 *  A dictionary consists of a mapping of string keys onto TemplateVariant values.
 *  A key is searched starting with the dictionary at the top of the stack
 *  and searching downwards until it is found. The stack is used to create
 *  local scopes.
 *  @note This object must be created by TemplateEngine
 */
class TemplateContext
{
  public:
    virtual ~TemplateContext() {}

    /** Push a new scope on the stack. */
    virtual void push() = 0;

    /** Pop the current scope from the stack. */
    virtual void pop() = 0;

    /** Sets a value in the current scope. 
     *  @param[in] name The name of the value; the key in the dictionary.
     *  @param[in] v The value associated with the key.
     *  @note When a given key is already present, 
     *  its value will be replaced by \a v
     */
    virtual void set(const char *name,const TemplateVariant &v) = 0;

    /** Gets the value for a given key
     *  @param[in] name The name of key.
     *  @returns The value, which can be an invalid variant in case the
     *  key was not found.
     */
    virtual TemplateVariant get(const QCString &name) const = 0;

    /** Returns a pointer to the value corresponding to a given key.
     *  @param[in] name The name of key.
     *  @returns A pointer to the value, or 0 in case the key was not found.
     */
    virtual const TemplateVariant *getRef(const QCString &name) const = 0;

    /** When files are create (i.e. by {% create ... %}) they written
     *  to the directory \a dir.
     */
    virtual void setOutputDirectory(const QCString &dir) = 0;

    /** Sets the interface that will be used for escaping the result
     *  of variable expansion before writing it to the output.
     */
    virtual void setEscapeIntf(TemplateEscapeIntf *intf) = 0;
};

//------------------------------------------------------------------------

/** @brief Abstract interface for a template. 
 *  @note Must be created by TemplateEngine
 */
class Template
{
  public:
    /** Destructor */
    virtual ~Template() {}

    /** Renders a template instance to a stream. 
     *  @param[in] ts The text stream to write the results to.
     *  @param[in] c The context containing data that can be used
     *  when instantiating the template.
     */
    virtual void render(FTextStream &ts,TemplateContext *c) = 0;
};

//------------------------------------------------------------------------

/** @brief Engine to create templates and template contexts. */
class TemplateEngine
{
  public:
    /** Create a template engine. */
    TemplateEngine();

    /** Destroys the template engine. */
   ~TemplateEngine();

    /** Creates a new context that can be using to render a template.
     *  @see Template::render()
     */
    TemplateContext *createContext() const;

    /** Creates a new template whose contents are given by a string.
     *  @param[in] name The name of the template.
     *  @param[in] data The contents of the template.
     *  @return the new template, the caller will be the owner.
     */
    Template *newTemplate(const QCString &name,const QCString &data);

    /** Creates a new template whole contents are in a file.
     *  @param[in] fileName The name of the file containing the 
     *             template data
     *  @return the new template, the caller will be the owner.
     */
    Template *loadByName(const QCString &fileName);

  private:
    class Private;
    Private *p;
};

/** @} */

#endif
