#include "tool/config_element.h"
#include "tool/xml_document.h"

bool IntConfigElement::Read(const xmlNode* father)
{
  if (!XmlReader::ReadInt(father, m_name, *m_val)) {
    *m_val = m_def;
    return false;
  }
  ASSERT(!m_important || (*m_val >= m_min && *m_val <= m_max));
  return true;
}
void IntConfigElement::Write(XmlWriter& writer, xmlNode* father) const
{
  writer.WriteElement(father, m_name, int2str(*m_val));
}

//----------------------------------------------------------------------

bool BoolConfigElement::Read(const xmlNode* father)
{
  if (!XmlReader::ReadBool(father, m_name, *m_val)) {
    *m_val = m_def;
    return false;
  }
  return true;
}

void BoolConfigElement::Write(XmlWriter& writer, xmlNode* father) const
{
  writer.WriteElement(father, m_name, bool2str(*m_val));
}

//----------------------------------------------------------------------

bool UintConfigElement::Read(const xmlNode* father)
{
  int val;
  if (!XmlReader::ReadInt(father, m_name, val) || val<0) {
    *m_val = m_def;
    return false;
  }
  ASSERT(!m_important || (*m_val >= m_min && *m_val <= m_max));
  *m_val = val;
  return true;
}
void UintConfigElement::Write(XmlWriter& writer, xmlNode* father) const
{
  writer.WriteElement(father, m_name, uint2str(*m_val));
}

//----------------------------------------------------------------------

bool DoubleConfigElement::Read(const xmlNode* father)
{
  if (!XmlReader::ReadDouble(father, m_name, *m_val)) {
    *m_val = m_def;
    return false;
  }
  ASSERT(!m_important || (*m_val >= m_min && *m_val <= m_max));
  return true;
}
void DoubleConfigElement::Write(XmlWriter& writer, xmlNode* father) const
{
  writer.WriteElement(father, m_name, Double2str(*m_val));
}

//----------------------------------------------------------------------

bool StringConfigElement::Read(const xmlNode* father)
{
  if (!XmlReader::ReadString(father, m_name, *m_val)) {
    *m_val = m_def;
    return false;
  }
  return true;
}
void StringConfigElement::Write(XmlWriter& writer, xmlNode* father) const
{
  writer.WriteElement(father, m_name, *m_val);
}

ConfigElementList::~ConfigElementList()
{
  for (iterator it = begin(); it != end(); ++it)
    delete (*it);
}

void ConfigElementList::LoadXml(const xmlNode* elem, const char* node)
{
  if (node && elem)
    elem = XmlReader::GetMarker(elem, node);
  if (!elem)
    return;
  for (iterator it = begin(); it != end(); ++it)
    (*it)->Read(elem);
}

xmlNode* ConfigElementList::SaveXml(XmlWriter& writer, xmlNode* elem, const char* node)
{
  if (node && elem)
    elem = XmlWriter::AddNode(elem, node);
  if (!elem)
    return NULL;
  for (const_iterator it = begin(); it != end(); ++it)
    (*it)->Write(writer, elem);
  return elem;
}
