#ifndef CONFIG_ELEMENT_H
#define CONFIG_ELEMENT_H

#include <list>

#include <WARMUX_base.h>
#include <WARMUX_error.h>

class XmlWriter;
typedef struct _xmlNode xmlNode;

class ConfigElement
{
public:
  typedef enum
  {
    TYPE_INT,
    TYPE_UINT,
    TYPE_BOOL,
    TYPE_DOUBLE
  } Type;

  Type        m_type;
  const char *m_name;
  bool        m_important;

  virtual bool Read(const xmlNode* father) const = 0;
  virtual void Write(XmlWriter& writer, xmlNode* elem) const = 0;

protected:
  ConfigElement(Type t, const char *n, bool imp = false)
  {
    m_type = t; m_name = n; m_important = imp;
  }
};

class IntConfigElement : public ConfigElement
{
public:
  int       *m_val, m_def, m_min, m_max;
  IntConfigElement(const char *n, int *v, int d)
    : ConfigElement(TYPE_INT, n, false) {  m_val = v; *v = m_def = d; }
  IntConfigElement(const char *n, int *v, int d, int mi, int ma)
    : ConfigElement(TYPE_INT, n, true) { m_val = v; *v = m_def = d; m_min = mi, m_max = ma; ASSERT(d >= mi && d <= ma); }
  bool Read(const xmlNode* father) const;
  void Write(XmlWriter& writer, xmlNode* father) const;
};

class BoolConfigElement : public ConfigElement
{
public:
  bool       *m_val, m_def;
  BoolConfigElement(const char *n, bool *v, bool d, bool imp = false)
    : ConfigElement(TYPE_BOOL, n, imp) { m_val = v; *v = m_def = d; }
  bool Read(const xmlNode* father) const;
  void Write(XmlWriter& writer, xmlNode* father) const;
};

class UintConfigElement : public ConfigElement
{
public:
  uint       *m_val, m_def, m_min, m_max;
  UintConfigElement(const char *n, uint *v, uint d)
    : ConfigElement(TYPE_UINT, n, false) { m_val = v; *v = m_def = d; }
  UintConfigElement(const char *n, uint *v, uint d, uint mi, uint ma)
    : ConfigElement(TYPE_UINT, n, true) { m_val = v; *v = m_def = d; m_min = mi, m_max = ma; ASSERT(d >= mi && d <= ma); }
  bool Read(const xmlNode* father) const;
  void Write(XmlWriter& writer, xmlNode* father) const;
};

class DoubleConfigElement : public ConfigElement
{
public:
  Double       *m_val, m_def, m_min, m_max;
  DoubleConfigElement(const char *n, Double* v, Double d)
    : ConfigElement(TYPE_DOUBLE, n, false) { m_val = v; *v = m_def = d; }
  DoubleConfigElement(const char *n, Double* v, Double d, Double mi, Double ma)
    : ConfigElement(TYPE_DOUBLE, n, true) { m_val = v; *v = m_def = d; m_min = mi, m_max = ma; ASSERT(d >= mi && d <= ma); }
  virtual bool Read(const xmlNode* father) const;
  virtual void Write(XmlWriter& writer, xmlNode* father) const;
};

class AngleConfigElement : public DoubleConfigElement
{
  static const Double ToDegree;
  static const Double ToRad;
public:
  AngleConfigElement(const char *n, Double* v, Double d) : DoubleConfigElement(n, v, false) { }
  AngleConfigElement(const char *n, Double* v, Double d, Double mi, Double ma)
    : DoubleConfigElement(n, v, d*ToRad, mi*ToRad, ma*ToRad) { }
  virtual bool Read(const xmlNode* father) const;
  virtual void Write(XmlWriter& writer, xmlNode* father) const;
};

class StringConfigElement : public ConfigElement
{
public:
  std::string       *m_val, m_def;
  StringConfigElement(const char *n, std::string *v, const std::string& d, bool imp = false)
    : ConfigElement(TYPE_BOOL, n, imp) { m_val = v; *v = m_def = d; }
  bool Read(const xmlNode* father) const;
  void Write(XmlWriter& writer, xmlNode* father) const;
};

class ConfigElementList : public std::list<ConfigElement*>
{
  std::list<ConfigElementList*>  children;
  const char                    *node;

public:
  ConfigElementList() : node(NULL) { }
  virtual ~ConfigElementList();

  typedef std::list<ConfigElement*>::iterator iterator;
  typedef std::list<ConfigElement*>::const_iterator const_iterator;

  void LoadXml(const xmlNode* elem) const;
  xmlNode *SaveXml(XmlWriter& writer, xmlNode* elem) const;

  void LinkList(ConfigElementList* child, const char *name)
  {
    child->node = name;
    children.push_back(child);
  }
};

#endif // CONFIG_ELEMENT_H
