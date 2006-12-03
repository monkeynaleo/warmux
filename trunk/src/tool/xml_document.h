/*
 * Exemple d'utilisation de la librairie libxml++ version 1.0.
 */

#ifndef XML_DOCUMENT_H
#define XML_DOCUMENT_H

#include "../include/base.h"
#include <string>
#include <libxml++/libxml++.h>

class LitDocXml
{
public:
  xmlpp::DomParser parser;

public:
  // Charge un document XML
  bool Charge(const std::string &nomfich);

  // Le document a ��correctement charg�?
  bool EstOk() const;

  // Lit la racine
  xmlpp::Element* racine() const;

  // Lit la valeur d'une balise
  static bool LitString (const xmlpp::Node *pere,
			 const std::string &nom,
			 std::string &sortie);
  static bool LitDouble (const xmlpp::Node *pere,
		      const std::string &nom,
		      double &sortie);
  static bool LitInt (const xmlpp::Node *pere,
		      const std::string &nom,
		      int &sortie);
  static bool LitUint (const xmlpp::Node *pere,
		      const std::string &nom,
		      unsigned int &sortie);
  static bool LitBool (const xmlpp::Node *pere,
		       const std::string &nom,
		       bool &sortie);

  // Lit les diff�entes valeurs d'une balise
  static bool LitListeString (const xmlpp::Node *x, 
					 const std::string &nom,
					 std::list<std::string> &sortie);

  // Lit la valeur d'une balise
  static bool LitValeurBalise (const xmlpp::Node *balise,
			       std::string &sortie);

  // Acc� �une balise
  static xmlpp::Element* AccesBalise (const xmlpp::Node *x,
				      const std::string &nom);

  // Access to the 'anchor' <[name] name="[attr_name]"> : have to be uniq !
  static xmlpp::Element* Access (const xmlpp::Node *x,
				 const std::string &name,
				 const std::string &attr_name);

  // Lit un attribut d'un noeud
  static bool LitAttrString (const xmlpp::Element *x, 
			     const std::string &nom, 
			     std::string &sortie);
  static bool LitAttrDouble (const xmlpp::Element *x, 
			  const std::string &nom, 
			  double &sortie);
  static bool LitAttrInt (const xmlpp::Element *x, 
			  const std::string &nom, 
			  int &sortie);
  static bool LitAttrUint (const xmlpp::Element *x, 
			  const std::string &nom, 
			  unsigned int &sortie);
  static bool LitAttrBool (const xmlpp::Element *x,
			  const std::string &nom,
			  bool &sortie);
};

//-----------------------------------------------------------------------------

class XmlWriter
{
protected:
  xmlpp::Document *m_doc;
  xmlpp::Element *m_root;
  std::string m_filename;
  bool m_save;
  std::string m_encoding;

public:
  XmlWriter();
  ~XmlWriter();

  bool Create(const std::string &filename, const std::string &root,
              const std::string &version, const std::string &encoding);

  bool IsOk() const;

  xmlpp::Element* GetRoot();

  void WriteElement(xmlpp::Element *x,
                    const std::string &name,
                    const std::string &value);

  bool Save();
};

#endif /* XML_DOCUMENT_H */
