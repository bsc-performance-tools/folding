/*****************************************************************************\
 *                        ANALYSIS PERFORMANCE TOOLS                         *
 *                                   Folding                                 *
 *              Instrumentation package for parallel applications            *
 *****************************************************************************
 *     ___     This library is free software; you can redistribute it and/or *
 *    /  __         modify it under the terms of the GNU LGPL as published   *
 *   /  /  _____    by the Free Software Foundation; either version 2.1      *
 *  /  /  /     \   of the License, or (at your option) any later version.   *
 * (  (  ( B S C )                                                           *
 *  \  \  \_____/   This library is distributed in hope that it will be      *
 *   \  \__         useful but WITHOUT ANY WARRANTY; without even the        *
 *    \___          implied warranty of MERCHANTABILITY or FITNESS FOR A     *
 *                  PARTICULAR PURPOSE. See the GNU LGPL for more details.   *
 *                                                                           *
 * You should have received a copy of the GNU Lesser General Public License  *
 * along with this library; if not, write to the Free Software Foundation,   *
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA          *
 * The GNU LEsser General Public License is contained in the file COPYING.   *
 *                                 ---------                                 *
 *   Barcelona Supercomputing Center - Centro Nacional de Supercomputacion   *
\*****************************************************************************/

#include "common.H"

#include "model.H"

#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <iostream>

#include <assert.h>

#define TAG_MODEL ((xmlChar*) "model")
#define TAG_COMPONENT ((xmlChar*) "component")
#define TAG_MODEL_NAME ((xmlChar*) "name")
#define TAG_COMPONENT_HIDDEN ((xmlChar*) "hidden")
#define TAG_COMPONENT_NAME TAG_MODEL_NAME
#define TAG_COMPONENT_WHERE ((xmlChar*) "where")
#define TAG_OPERATION ((xmlChar*) "operation")
#define TAG_VALUE ((xmlChar*) "value")
#define TAG_OPERATION_TYPE ((xmlChar *) "type")
#define TAG_MODEL_Y1 ((xmlChar*) "y1")
#define TAG_MODEL_Y2 ((xmlChar*) "y2")
#define TAG_MODEL_Y1STACKED ((xmlChar*) "y1-stacked")
#define TAG_TITLE_NAME ((xmlChar*) "title-name")
#define TAG_COLOR ((xmlChar*) "color")

/* Free memory if not null */
#define XML_FREE(ptr) \
	if (ptr != NULL) xmlFree(ptr);

#define is_Whitespace(c) \
	((c) == ' ' || (c) == '\t' || (c) == '\v' || (c) == '\f' || (c) == '\n')

static xmlChar * xmlChar_strip (xmlChar *str)
{
	xmlChar *tmp;
	int i;
	int initial = 0;
	int sublen = 0;
	int length = xmlStrlen (str);
	int end = length;

	/* First get rid of the leading and trailing white spaces */
	for (i = 0; i < length; i++)
		if (!is_Whitespace (str[i]))
			break;
	initial = i;
	for (; end-1 >= i; end--)
		if (!is_Whitespace (str[end-1]))
			break;

	sublen = end - initial;

	tmp = xmlStrsub (str, initial, sublen);

	return tmp;
}

static vector<xmlNodePtr> getOperationAndValueTags (xmlNodePtr tag)
{
	vector<xmlNodePtr> v;

	while (tag != NULL)
	{
		if (!xmlStrcasecmp (tag->name, TAG_OPERATION) ||
		    !xmlStrcasecmp (tag->name, TAG_VALUE))
			v.push_back (tag);

		tag = tag->next;
	}
	return v;
}


Model::Model ()
{
}

Model::~Model ()
{
	vector<ComponentModel*>:: iterator it = components.begin();
	while (it != components.end())
	{
		ComponentModel *ptr = *it;
		it = components.erase (it);
		delete ptr;
	}
}

bool Model::loadXML (char *f)
{
	if (!common::existsFile (f))
		return false;

	/*
	* This initialize the library and check potential ABI mismatche
	* between the version it was compiled for and the actual shared
	* library used.
	*/
	LIBXML_TEST_VERSION;

	xmlDocPtr xmldoc = xmlParseFile (f);
	if (xmldoc != NULL)
	{
		xmlNodePtr root_tag = xmlDocGetRootElement (xmldoc);

		while (root_tag != NULL)
		{

			if (!xmlStrcasecmp(root_tag->name, TAG_MODEL))
			{
				xmlChar *name = xmlGetProp (root_tag, TAG_MODEL_NAME);
				if (name == NULL)
				{
					cerr << "Missing attribute '" << TAG_MODEL_NAME << "' in <" <<
					  TAG_MODEL << ">." << endl;
					exit (-1);
				}
				xmlChar *y1 = xmlGetProp (root_tag, TAG_MODEL_Y1);
				if (y1 == NULL)
				{
					cerr << "Missing attribute '" << TAG_MODEL_Y1 << "' in <" <<
					  TAG_MODEL << ">." << endl;
					exit (-1);
				}
				xmlChar *y2 = xmlGetProp (root_tag, TAG_MODEL_Y2);
				if (y2 == NULL)
				{
					cerr << "Missing attribute '" << TAG_MODEL_Y2 << "' in <" <<
					  TAG_MODEL << ">." << endl;
					exit (-1);
				}
				xmlChar *titlename = xmlGetProp (root_tag, TAG_TITLE_NAME);
				if (titlename == NULL)
				{
					cerr << "Missing attribute '" << TAG_TITLE_NAME << "' in <" <<
					  TAG_MODEL << ">." << endl;
					exit (-1);
				}
				Name = string ((const char*) name);
				Y1AxisName = string ((const char*) y1);
				Y2AxisName = string ((const char*) y2);
				Y1Stacked = !xmlStrcasecmp (xmlGetProp (root_tag, TAG_MODEL_Y1STACKED), (const xmlChar*) "yes");
				TitleName = string ((const char*) titlename);
				XML_FREE(y2);
				XML_FREE(y1);
				XML_FREE(name);

				xmlNodePtr tag = root_tag->xmlChildrenNode;
				while (tag != NULL)
				{
					if (!xmlStrcasecmp (tag->name, TAG_COMPONENT))
						components.push_back ( loadXML_component (xmldoc, tag) );
					tag = tag->next;
				}
			}
			root_tag = root_tag->next;
		}
	}

	return components.size() > 0;
}

ComponentModel * Model::loadXML_component (xmlDocPtr xmldoc, xmlNodePtr tag)
{
	xmlChar *cname = xmlGetProp (tag, TAG_COMPONENT_NAME);
	if (cname == NULL)
	{
		cerr << "Missing attribute '" << TAG_COMPONENT_NAME << "' in <" <<
		  TAG_COMPONENT << ">." << endl;
		exit (-1);
	}
	string componentname = string((const char*)cname);
	XML_FREE(cname);

	xmlChar *titlename = xmlGetProp (tag, TAG_TITLE_NAME);
	if (titlename == NULL)
	{
		cerr << "Missing attribute '" << TAG_TITLE_NAME << "' in <" <<
		  TAG_COMPONENT << ">." << endl;
		exit (-1);
	}
	string componenttitlename = string((const char*)titlename);
	XML_FREE(titlename);

	xmlChar *position = xmlGetProp (tag, TAG_COMPONENT_WHERE);
	if (position == NULL)
	{
		cerr << "Missing attribute '" << TAG_COMPONENT_WHERE <<"' in <" <<
		  TAG_COMPONENT << ">." << endl;
		exit (-1);
	}
	string plotlocation = string ((const char*) position);
	if (plotlocation != "y1" && plotlocation != "y2")
	{
		cerr << "Invalid plot location " << plotlocation << endl;
		exit (-1);
	}
	XML_FREE(position);

	xmlChar *color = xmlGetProp (tag, TAG_COLOR);
	string plotcolor;
	if (color != NULL)
		if (strlen((const char*)color) > 0)
			plotcolor = string ((const char*) color);
	XML_FREE(color);

	vector<xmlNodePtr> v = getOperationAndValueTags (tag->xmlChildrenNode);
	assert (v.size() == 1);

	ComponentNode *cn = loadXML_component_componentnode (xmldoc, v[0]);
	if (common::DEBUG())
	{
		cout << "Definition for component " << componentname << 
		  " within model " << Name << endl;
		cn->show();
	}

	bool isHidden = false;
	xmlChar *hidden = xmlGetProp (tag, TAG_COMPONENT_HIDDEN);
	if (hidden != NULL)
	{
		isHidden = !xmlStrcasecmp (hidden, (const xmlChar*)"yes");
		XML_FREE(hidden);
	}

	return new ComponentModel (componentname, componenttitlename, plotlocation,
	  plotcolor, cn, isHidden);
}

ComponentNode * Model::loadXML_component_componentnode (xmlDocPtr xmldoc,
	xmlNodePtr tag, unsigned depth)
{
	ComponentNode *res = NULL;

	if (!xmlStrcasecmp (tag->name, TAG_OPERATION))
	{
		xmlChar *op = xmlGetProp (tag, TAG_OPERATION_TYPE);
		if (op == NULL)
		{
			cerr << "Missing attribute '" << TAG_OPERATION_TYPE << "' in <" <<
			  TAG_OPERATION << ">." << endl;
			exit (-1);
		}

		ComponentNode_derived::Operator OP;
		if (op[0] != '+' && op[0] != '-' && op[0] != '*' && op[0] != '/' && op[0] != 'm' && op[0] != 'M')
		{
			cerr << "Invalid operation type in <" << TAG_OPERATION << ">." << endl;
			exit (-1);
		}
		else
		{
			switch (op[0])
			{
				case '+': OP = ComponentNode_derived::ADD;
				break;
				case '-': OP = ComponentNode_derived::SUB;
				break;
				case '*': OP = ComponentNode_derived::MUL;
				break;
				case '/': OP = ComponentNode_derived::DIV;
				break;
				case 'm': OP = ComponentNode_derived::MIN;
				break;
				case 'M': OP = ComponentNode_derived::MAX;
				break;
				default:  OP = ComponentNode_derived::NOP;
				break;
			}
		}

		vector<xmlNodePtr> v = getOperationAndValueTags (tag->xmlChildrenNode);
		assert (v.size() == 2);

		res = new ComponentNode_derived (OP,
		  loadXML_component_componentnode (xmldoc, v[0], depth+1),
		  loadXML_component_componentnode (xmldoc, v[1], depth+1));

		XML_FREE (op);
	}
	else if (!xmlStrcasecmp (tag->name, TAG_VALUE))
	{
		vector<xmlNodePtr> v = getOperationAndValueTags (tag->xmlChildrenNode);
		assert (v.size() == 0);

		xmlChar *str = xmlChar_strip(xmlNodeListGetString (xmldoc, tag->xmlChildrenNode, 1));
		if (str == NULL)
		{
			cerr << "Missing value name in '" << TAG_VALUE << "'" << std::endl;
			exit(-1);
		}
		string s = (const char*) str;
		if (s.length() == 0)
		{
			cerr << "Missing value name in '" << TAG_VALUE << "'" << std::endl;
			exit(-1);
		}

		// Ok this is a value, but is this a number or anything else (counter)?
		char *pend;
		double d = strtod (s.c_str(), &pend);
		if (pend == &(s.c_str())[s.length()])
		{
			res = new ComponentNode_constant (d);
		}
		else
		{
			// Check whether the given value corresponds to an existing component.
			// In case of a positive match, we create an alias.
			ComponentModel *cn = NULL;
			for (const auto c : components)
			{
				if (c->getName() == s)
				{
					cn = c;
					break;
				}
			}
			if (cn != NULL)
			{
				res = new ComponentNode_alias (cn);
			}
			else
			{
				res = new ComponentNode_data (s);
			}
		}

		XML_FREE(str);
	}

	return res;
}

set<string> Model::requiredCounters (void) const
{
	set<string> res;
	for (unsigned c = 0; c < components.size(); c++)
	{
		set<string> cCounters = components[c]->requiredCounters();
		res.insert (cCounters.begin(), cCounters.end());
	}
	return res;
}
