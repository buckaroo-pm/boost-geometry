// Boost.Geometry (aka GGL, Generic Geometry Library)
//
// Copyright (c) 2010-2012 Barend Gehrels, Amsterdam, the Netherlands.
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
//
#ifndef QUICKBOOK_OUTPUT_HPP
#define QUICKBOOK_OUTPUT_HPP


#include <string>
#include <vector>

#include <boost/algorithm/string.hpp>

#include <doxygen_elements.hpp>
#include <parameter_predicates.hpp>

std::string qbk_escaped(std::string const& s)
{
    // Replace _ by unicode to avoid accidental quickbook underlining.
    // 1) do NOT do this in quickbook markup, so not within []
    // (e.g. to avoid [include get_point.qbk] having unicoded)
    // 2) \[ and \] should not count as []
    std::size_t const len = s.length();
    int counter = 0;
    std::string result = "";
    for (std::size_t i = 0; i < len; i++)
    {
        switch(s[i])
        {
            case '[' : counter++; break;
            case ']' : counter--; break;
            case '\\' : 
                {
                    result += s[i];
                    if (i + 1 < len)
                    {
                        result += s[i + 1];
                    }
                    i++;
                    continue;
                }
            case '_' : 
                if (counter == 0)
                {
                    result += "\\u005f";
                    continue;
                }
        }
        result += s[i];
    }

    return result;
}



void quickbook_template_parameter_list(std::vector<parameter> const& parameters, std::ostream& out, bool multiline = false, bool show_defaults = true)
{
    std::string next_param;
    if ( multiline )
    {
        if ( 2 < parameters.size() )
            next_param = std::string(",\n") + "         ";
        else
            next_param = ", ";
    }
    else
        next_param = ", ";
        

    if (!parameters.empty())
    {
        out << "template<" ;
        bool first = true;
        BOOST_FOREACH(parameter const& p, parameters)
        {
            out << (first ? "" : next_param.c_str()) << p.fulltype;

            if ( show_defaults && !p.default_value.empty() )
                out << " = " << p.default_value;

            first = false;
        }
        out << ">" << std::endl;
    }
}


void quickbook_synopsis(function const& f, std::ostream& out, bool multiline = false, bool show_defaults = true)
{
    out << "``";
    quickbook_template_parameter_list(f.template_parameters, out, multiline, show_defaults);

    unsigned offset = 1; // '('
    switch(f.type)
    {
        case function_constructor_destructor :
            out << f.name;
            offset += f.name.size();
            break;
        case function_member :
            out << f.return_type << " " << f.name;
            offset += f.return_type.size() + 1 + f.name.size();
            break;
        case function_free :
            out << f.definition;
            offset += f.definition.size();
            break;
        case function_define :
            out << "#define " << f.name;
            offset += 8 + f.name.size();
            break;
        case function_unknown :
            // do nothing
            break;
    }

    std::string after_parameter(", ");
    if ( multiline && 2 < f.parameters.size() )
        after_parameter = std::string(",\n") + std::string(offset, ' ');

    // Output the parameters
    // Because we want to be able to skip, we cannot use the argstring
    {
        bool first = true;
        BOOST_FOREACH(parameter const& p, f.parameters)
        {
            if (! p.skip)
            {
                out 
                    << (first ? "(" : after_parameter)
                    << p.fulltype << (p.fulltype.empty() ? "" : " ")
                    << p.name
                    << (p.default_value.empty() ? "" : " = ")
                    << p.default_value;
                first = false;
            }
        }
        if (! first)
        {
            out << ")";
        }
        else if (f.type != function_define)
        {
            out << "()";
        }
    }

    out << "``" 
        << std::endl
        << std::endl;
}


void quickbook_synopsis(enumeration const& e, std::ostream& out)
{
    out << "``enum " << e.name;
    bool first = true;
    BOOST_FOREACH(enumeration_value const& value, e.enumeration_values)
    {
        out << (first ? " {" : ", ") << value.name;
        if (! value.initializer.empty())
        {
            out << " = " << boost::trim_copy(value.initializer);
        }
        first = false;
    }
    if (! first)
    {
        out << "};";
    }
    out << "``" 
        << std::endl
        << std::endl;
}

inline bool includes(std::string const& filename, std::string const& header)
{
    std::string result;

    std::ifstream cpp_file(filename.c_str());
    if (cpp_file.is_open())
    {
        while (! cpp_file.eof() )
        {
            std::string line;
            std::getline(cpp_file, line);
            boost::trim(line);
            if (boost::starts_with(line, "#include")
                && boost::contains(line, header))
            {
                return true;
            }
        }
    }
    return false;
}


void quickbook_header(std::string const& location,
    configuration const& config,
    std::ostream& out)
{
    if (! location.empty())
    {
        std::vector<std::string> including_headers;

        // Select headerfiles containing to this location
        BOOST_FOREACH(std::string const& header, config.convenience_headers)
        {
            if (includes(config.convenience_header_path + header, location))
            {
                including_headers.push_back(header);
            }
        }

        out << "[heading Header]" << std::endl;
        if (! including_headers.empty())
        {
            out << "Either"
                << (including_headers.size() > 1 ? " one of" : "")
                << std::endl << std::endl;
            BOOST_FOREACH(std::string const& header, including_headers)
            {
                out << "`#include <" << config.start_include << header << ">`" << std::endl;
            }

            out << std::endl << "Or" << std::endl << std::endl;
        }
        out << "`#include <" << location << ">`" << std::endl;
        out << std::endl;
    }
}


void quickbook_markup(std::vector<markup> const& qbk_markup, 
            markup_order_type order, markup_type type, 
            std::ostream& out)
{
    bool has_output = false;
    BOOST_FOREACH(markup const& inc, qbk_markup)
    {
        if (inc.type == type && inc.order == order)
        {
            out << inc.value << std::endl;
            has_output = true;
        }
    }
    if (has_output)
    {
        out << std::endl;
    }
}



void quickbook_string_with_heading_if_present(std::string const& heading,
            std::string const& contents, std::ostream& out)
{
    if (! contents.empty())
    {
        out << "[heading " << heading << "]" << std::endl
            << qbk_escaped(contents) << std::endl
            << std::endl;
    }
}

inline std::string to_section_name(std::string const& name)
{
    // Make section-name lowercase and remove :: because these are filenames
    return boost::to_lower_copy(boost::replace_all_copy(name, "::", "_"));
}



void quickbook_short_output(function const& f, std::ostream& out)
{
    BOOST_FOREACH(parameter const& p, f.parameters)
    {
        if (! p.skip)
        {
            out << "[* " << p.fulltype << "]: ['" << p.name << "]:  " << p.brief_description << std::endl << std::endl;
        }
    }
    out << std::endl;
    out << std::endl;

    if (! f.return_description.empty())
    {
        out << "][" << std::endl;
        out << f.return_description << std::endl;
        out << std::endl;
    }

    out << std::endl;
}

inline std::string namespace_skipped(std::string const& name, configuration const& config)
{
    return config.skip_namespace.empty()
        ? name
        : boost::replace_all_copy(name, config.skip_namespace, "")
        ;
}

inline std::string output_if_different(std::string const& s, std::string const& s2)
{
    return boost::equals(s, s2)
        ? ""
        : s + " "
        ;
}

inline void quickbook_output_indexterm(std::string const& term, std::ostream& out
            //, std::string const& secondary = ""
            )
{
    out << "'''";
    if (boost::contains(term, "::"))
    {
        // "Unnamespace" it and add all terms (also namespaces)
        std::vector<std::string> splitted;
        std::string for_split = boost::replace_all_copy(term, "::", ":");
        boost::split(splitted, for_split, boost::is_any_of(":"), boost::token_compress_on);
        BOOST_FOREACH(std::string const& part, splitted)
        {
            out << "<indexterm><primary>" << part << "</primary></indexterm>";
        }
    }
    else
    {
        out << "<indexterm><primary>" << term;
        /*if (! secondary.empty())
        {
            out << "<secondary>" << secondary << "</secondary>";
        }*/
        out << "</primary></indexterm>";
    }
    out << "'''" << std::endl;
}

void quickbook_output(function const& f, configuration const& config, std::ostream& out)
{
    // Write the parsed function
    int arity = (int)f.parameters.size();

    std::string additional_description;

    if (! f.additional_description.empty())
    {
        additional_description = " (";
        additional_description += f.additional_description;
        additional_description += ")";
    }

    out << "[section:" << to_section_name(f.name);
    // Make section name unique if necessary by arity and additional description
    if (! f.unique)
    {
        out << "_" << arity;
        if (! f.additional_description.empty())
        {
            out << "_" << boost::to_lower_copy(boost::replace_all_copy(f.additional_description, " ", "_"));
        }
    }
    out << " " << f.name
        << additional_description
        << "]" << std::endl
        << std::endl;

    quickbook_output_indexterm(f.name, out);
        
    out << qbk_escaped(f.brief_description) << std::endl;
    out << std::endl;

    quickbook_string_with_heading_if_present("Description", f.detailed_description, out);

    // Synopsis
    quickbook_markup(f.qbk_markup, markup_before, markup_synopsis, out);
    out << "[heading Synopsis]" << std::endl;
    quickbook_synopsis(f, out);
    quickbook_markup(f.qbk_markup, markup_after, markup_synopsis, out);


    out << "[heading Parameters]" << std::endl
        << std::endl;

    out << "[table" << std::endl << "[";
    if (f.type != function_define)
    {
        out << "[Type] [Concept] ";
    }
    out << "[Name] [Description] ]" << std::endl;

    // First: output any template parameter which is NOT used in the normal parameter list
    BOOST_FOREACH(parameter const& tp, f.template_parameters)
    {
        if (! tp.skip)
        {
            std::vector<parameter>::const_iterator it = std::find_if(f.parameters.begin(), f.parameters.end(), par_by_type(tp.name));

            if (it == f.parameters.end())
            {
                out << "[[" << tp.name << "] [" << tp.brief_description << "] [ - ] [Must be specified]]" << std::endl;
            }
        }
    }

    BOOST_FOREACH(parameter const& p, f.parameters)
    {
        if (! p.skip)
        {
            out << "[";
            std::vector<parameter>::const_iterator it = std::find_if(f.template_parameters.begin(),
                f.template_parameters.end(), par_by_name(p.type));

            if (f.type != function_define)
            {
                out << "[" << p.fulltype
                    << "] [" << (it == f.template_parameters.end() ? "" : it->brief_description)
                    << "] ";
            }
            out << "[" << p.name
                << "] [" << p.brief_description
                << "]]"
                << std::endl;
        }
    }
    out << "]" << std::endl
        << std::endl
        << std::endl;

    quickbook_string_with_heading_if_present("Returns", f.return_description, out);

    quickbook_header(f.location, config, out);
    quickbook_markup(f.qbk_markup, markup_any, markup_default, out);

    out << std::endl;
    out << "[endsect]" << std::endl;
    out << std::endl;
}

void quickbook_output(enumeration const& e, configuration const& config, std::ostream& out)
{
    out << "[section:" << to_section_name(e.name);
    out << " " << e.name
        << "]" << std::endl
        << std::endl;

    quickbook_output_indexterm(e.name, out);
    BOOST_FOREACH(enumeration_value const& value, e.enumeration_values)
    {
        quickbook_output_indexterm(value.name, out);
    }

    out << e.brief_description << std::endl;
    out << std::endl;

    quickbook_string_with_heading_if_present("Description", e.detailed_description, out);

    // Synopsis
    quickbook_markup(e.qbk_markup, markup_before, markup_synopsis, out);
    out << "[heading Synopsis]" << std::endl;
    quickbook_synopsis(e, out);
    quickbook_markup(e.qbk_markup, markup_after, markup_synopsis, out);


    out << "[heading Values]" << std::endl
        << std::endl;

    out << "[table" << std::endl << "[";
    out << "[Value] [Description] ]" << std::endl;

    BOOST_FOREACH(enumeration_value const& value, e.enumeration_values)
    {
        out << "[[" << value.name
            << "] [" << value.brief_description
            << "]]"
            << std::endl;
    }
    out << "]" << std::endl
        << std::endl
        << std::endl;

    quickbook_header(e.location, config, out);
    quickbook_markup(e.qbk_markup, markup_any, markup_default, out);

    out << std::endl;
    out << "[endsect]" << std::endl;
    out << std::endl;
}

void quickbook_output_function(std::vector<function> const& functions, 
        function_type type, 
        std::string const& title,
        configuration const& config, std::ostream& out)
{
    std::string returns = type == function_constructor_destructor ? "" : " [Returns]";
    out << "[heading " << title << "(s)]" << std::endl
        << "[table" << std::endl
        << "[[Function] [Description] [Parameters] " << returns << "]" << std::endl;

    BOOST_FOREACH(function const& f, functions)
    {
        if (f.type == type)
        {
            out << "[[";
            quickbook_synopsis(f, out);
            out << "] [" << f.brief_description << "] [";
            quickbook_short_output(f, out);
            out << "]]" << std::endl;
        }
    }
    out << "]" << std::endl
        << std::endl;
}

void quickbook_output(class_or_struct const& cos, configuration const& config, std::ostream& out)
{
    // Skip namespace
    std::string short_name = namespace_skipped(cos.fullname, config);

    // Write the parsed function
    out << "[section:" << to_section_name(short_name) << " " << short_name << "]" << std::endl
        << std::endl;

    quickbook_output_indexterm(short_name, out);

    out << cos.brief_description << std::endl;
    out << std::endl;

    quickbook_string_with_heading_if_present("Description", cos.detailed_description, out);

    quickbook_markup(cos.qbk_markup, markup_before, markup_synopsis, out);
    out << "[heading Synopsis]" << std::endl
        << "``";
    quickbook_template_parameter_list(cos.template_parameters, out, false);
    out << (cos.is_class ? "class" : "struct")
        << " " << short_name << std::endl;

    if (! cos.base_classes.empty())
    {
        out << "      : ";
        bool first = true;
        BOOST_FOREACH(base_class const& bc, cos.base_classes)
        {
            if (! first)
            {
                out << std::endl << "      , ";
            }
            out << output_if_different(bc.derivation, "private")
                << output_if_different(bc.virtuality, "non-virtual")
                << namespace_skipped(bc.name, config);
            first = false;
        }
        out << std::endl;
    }

    out << "{" << std::endl
        << "  // ..." << std::endl
        << "};" << std::endl
        << "``" << std::endl << std::endl;
    quickbook_markup(cos.qbk_markup, markup_after, markup_synopsis, out);



    if (! cos.template_parameters.empty())
    {
        bool has_default = false;
        BOOST_FOREACH(parameter const& p, cos.template_parameters)
        {
            if (! p.default_value.empty())
            {
                has_default = true;
            }
        }

        out << "[heading Template parameter(s)]" << std::endl
            << "[table" << std::endl
            << "[[Parameter]";
        if (has_default)
        {
            out << " [Default]";
        }
        out << " [Description]]" << std::endl;

        BOOST_FOREACH(parameter const& p, cos.template_parameters)
        {
            out << "[[``";
            if ( p.fulltype.find("typename") == 0 )
                out << p.fulltype.substr(8);
            else if ( p.fulltype.find("class") == 0 )
                out << p.fulltype.substr(5);
            else
                out << p.fulltype;

            out << "``]";

            if (has_default && !p.default_value.empty())
                out << "[``" << p.default_value << "``]";
            else
                out << "[]";

            out << " [" << p.brief_description << "]]" << std::endl;
        }
        out << "]" << std::endl
            << std::endl;
    }


    std::map<function_type, int> counts;
    BOOST_FOREACH(function const& f, cos.functions)
    {
        counts[f.type]++;
    }

    if (counts[function_constructor_destructor] > 0)
    {
        quickbook_output_function(cos.functions, function_constructor_destructor, "Constructor", config, out);
    }

    if (counts[function_member] > 0)
    {
        quickbook_output_function(cos.functions, function_member, "Member Function", config, out);
    }

    quickbook_header(cos.location, config, out);
    quickbook_markup(cos.qbk_markup, markup_any, markup_default, out);

    out << "[endsect]" << std::endl
        << std::endl;
}





std::string remove_template_parameters(std::string const& name)
{
    std::string res;
    std::string::size_type prev_i = 0, i = 0;
    int blocks_counter = 0;
    for ( ;; )
    {
        std::string::size_type next_begin = name.find('<', i);
        std::string::size_type next_end = name.find('>', i);

        if ( next_begin == next_end )
        {
            res += name.substr(prev_i, next_begin);
            break;
        }
        else if ( next_begin < next_end )
        {
            i = next_begin + 1;
            if ( blocks_counter == 0 )
                res += name.substr(prev_i, next_begin) + "<...>";
            blocks_counter++;
        }
        else
        {
            i = next_end + 1;
            blocks_counter--;
            if ( blocks_counter == 0 )
                prev_i = i;
        }
    }

    return res;
}

std::string replace_brackets(std::string const& str)
{
    return boost::replace_all_copy(boost::replace_all_copy(str, "[", "\\["), "]", "\\]");
}

void quickbook_synopsis_short(function const& f, std::ostream& out)
{
    switch(f.type)
    {
    case function_constructor_destructor :
        out << f.name;
        break;
    case function_member :
        out << f.name;
        break;
    case function_free :
        out << f.name;
        break;
    case function_define :
        out << "#define " << f.name;
        break;
    case function_unknown :
        // do nothing
        break;
    }

    bool first = true;
    BOOST_FOREACH(parameter const& p, f.parameters)
    {
        if ( !p.skip && p.default_value.empty() )
        {
            out << (first ? "(" : ", ") << remove_template_parameters(p.fulltype);
            first = false;
        }
    }


    if (! first)
        out << ")";
    else if (f.type != function_define)
        out << "()";
}

void quickbook_output_function(std::vector<function> const& functions, 
                             function_type type,
                             configuration const& config,
                             std::string const& qbk_id_prefix,
                             std::ostream& out,
                             bool display_all = false)
{
    out << "[table" << std::endl
        << "[[Function][Description]]" << std::endl;

    for ( size_t i = 0 ; i < functions.size() ; ++i )
    {
        function const& f = functions[i];

        if (display_all || f.type == type)
        {
            out << "[[";
            if ( !config.index_id_path.empty() )
                out << "[link " << config.index_id_path
                    << "." << qbk_id_prefix << i << " ";
            out << "`";
            quickbook_synopsis_short(f, out);
            out << "`";
            if ( !config.index_id_path.empty() )
                out << "]";
            out << "][" << f.brief_description << "]]" << std::endl;
        }
    }
    out << "]" << std::endl
        << std::endl;
}

void output_paragraphs_note_warning(element const& el, std::ostream & out)
{
    // Additional paragraphs
    if ( !el.paragraphs.empty() )
    {
        BOOST_FOREACH(paragraph const& p, el.paragraphs)
        {
            if ( !p.title.empty() )
                out << "[heading " << p.title << "]" << std::endl;
            else
                out << "\n\n" << std::endl;
            out << p.text << std::endl;
            out << std::endl;
        }
    }

    // Note
    if ( !el.note.empty() )
    {
        out << "[note " << el.note << "]" << std::endl;
        out << std::endl;
    }

    // Warning
    if ( !el.warning.empty() )
    {
        out << "[warning " << el.warning << "]" << std::endl;
        out << std::endl;
    }
}

void quickbook_output_detail_function(std::vector<function> const& functions, 
                                    function_type type, 
                                    configuration const& config,
                                    std::string const& qbk_id_prefix,
                                    std::ostream& out,
                                    bool display_all = false)
{
    for ( size_t i = 0 ; i < functions.size() ; ++i )
    {
        function const& f = functions[i];
        if ( display_all || f.type == type )
        {
            // Section
            std::stringstream ss;
            quickbook_synopsis_short(f, ss);
            out << "[section:" << qbk_id_prefix << i << " " << replace_brackets(ss.str()) << "]" << std::endl;
            
            // Brief description
            out << f.brief_description << std::endl;
            out << std::endl;

            // Detail description
            if ( !f.detailed_description.empty() )
            {
                out << "[heading Description]" << std::endl;
                out << f.detailed_description;
            }

            // Synopsis
            quickbook_markup(f.qbk_markup, markup_before, markup_synopsis, out);
            out << "[heading Synopsis]" << std::endl;
            quickbook_synopsis(f, out, true, true);
            quickbook_markup(f.qbk_markup, markup_after, markup_synopsis, out);

            // Parameters
            if ( !f.parameters.empty() )
            {
                out << "[heading Parameter(s)]" << std::endl;
                out << "[table " << std::endl;
                out << "[[Type][Name][Description]]" << std::endl;
                BOOST_FOREACH(parameter const& p, f.parameters)
                {
                    if (!p.skip)
                    {
                        out << "[[ `" << p.fulltype << "` ][ `" << p.name << "` ][" << p.brief_description << "]]"<< std::endl;
                    }
                }
                out << "]" << std::endl;
            }

            // Precondition
            if ( !f.precondition.empty() )
            {
                out << "[heading Precondition(s)]" << std::endl;
                out << f.precondition << std::endl;
                out << std::endl;
            }

            // Return
            if ( !f.return_description.empty() )
            {
                out << "[heading Returns]" << std::endl;
                out << f.return_description << std::endl;
            }
            
            // Additional paragraphs, note, warning
            output_paragraphs_note_warning(f, out);

            // QBK markup
            quickbook_markup(f.qbk_markup, markup_any, markup_default, out);

            // Section end
            out << "[endsect]" << std::endl
                //<< "[br]" << std::endl
                << std::endl;
        }
    }
}

void quickbook_output_alt(documentation const& doc, configuration const& config, std::ostream& out)
{
    std::string id_prefix("function");

    if ( !doc.group_id.empty() )
    {
        id_prefix = doc.group_id + ".function";
        std::cout << "[section:" << doc.group_id << " " << doc.group_title << "]" << std::endl;
    }

    quickbook_output_function(doc.functions, function_constructor_destructor, config, id_prefix, out, true);
    quickbook_output_detail_function(doc.functions, function_unknown, config, "function", out, true);

    if ( !doc.group_id.empty() )
    {
        out << "[endsect]" << std::endl
            << std::endl;
    }    
}

template <typename Range>
bool has_brief_description(Range const& rng)
{
    typedef typename Range::value_type V;
    BOOST_FOREACH(V const& bc, rng)
    {
        if ( !bc.brief_description.empty() )
            return true;
    }
    return false;
}

template <typename Range>
bool has_brief_description(Range const& rng, function_type t)
{
    typedef typename Range::value_type V;
    BOOST_FOREACH(V const& bc, rng)
    {
        if ( bc.type == t && !bc.brief_description.empty() )
            return true;
    }
    return false;
}

void quickbook_output_alt(class_or_struct const& cos, configuration const& config, std::ostream& out)
{
    // Skip namespace
    std::string short_name = namespace_skipped(cos.fullname, config);
    std::string section_name = to_section_name(short_name);

    // Write the parsed function
    out << "[section:" << section_name << " " << short_name << "]" << std::endl
        << std::endl;

    quickbook_output_indexterm(short_name, out);

    // Brief

    out << cos.brief_description << std::endl;
    out << std::endl;

    // Description

    quickbook_string_with_heading_if_present("Description", cos.detailed_description, out);

    // Additional paragraphs, note, warning
    output_paragraphs_note_warning(cos, out);

    // Markup
    quickbook_markup(cos.qbk_markup, markup_any, markup_default, out);

    // Header

    quickbook_header(cos.location, config, out);

    // Class synposis

    quickbook_markup(cos.qbk_markup, markup_before, markup_synopsis, out);
    out << "[heading Synopsis]" << std::endl
        << "``";
    quickbook_template_parameter_list(cos.template_parameters, out, true, true);
    out << (cos.is_class ? "class " : "struct ");
    out << short_name.substr(short_name.find_last_of("::") + 1) << std::endl;

    if (! cos.base_classes.empty())
    {
        out << "      : ";
        bool first = true;
        BOOST_FOREACH(base_class const& bc, cos.base_classes)
        {
            if (! first)
            {
                out << std::endl << "      , ";
            }
            out << output_if_different(bc.derivation, "private")
                << output_if_different(bc.virtuality, "non-virtual")
                << namespace_skipped(bc.name, config);
            first = false;
        }
        out << std::endl;
    }

    out << "{" << std::endl
        << "  // ..." << std::endl
        << "};" << std::endl
        << "``" << std::endl << std::endl;
    quickbook_markup(cos.qbk_markup, markup_after, markup_synopsis, out);

    // Template parameters

    if (! cos.template_parameters.empty())
    {
        if ( has_brief_description(cos.template_parameters) )
        {
            out << "[heading Template parameter(s)]" << std::endl
                << "[table" << std::endl
                << "[[Parameter] [Description]]" << std::endl;

            BOOST_FOREACH(parameter const& p, cos.template_parameters)
            {
                if ( p.brief_description.empty() )
                    continue;

                out << "[[`";
                if ( p.fulltype.find("typename ") == 0 )
                    out << p.fulltype.substr(9);
                else if ( p.fulltype.find("class ") == 0 )
                    out << p.fulltype.substr(6);
                else
                    out << p.fulltype;
                out << "`][" << p.brief_description << "]]" << std::endl;
            }
            out << "]" << std::endl
                << std::endl;
        }
    }

    // Typedefs

    if ( !cos.typedefs.empty() )
    {
        if ( has_brief_description(cos.typedefs) )
        {
            out << "[heading Typedef(s)]" << std::endl
                << "[table" << std::endl
                << "[[Type]";
            out << " [Description]]" << std::endl;

            BOOST_FOREACH(base_element const& e, cos.typedefs)
            {
                if ( e.brief_description.empty() )
                    continue;

                out << "[[`" << e.name;
                out << "`][" << e.brief_description << "]]" << std::endl;
            }
            out << "]" << std::endl
                << std::endl;
        }
    }

    // Members

    bool display_ctors = has_brief_description(cos.functions, function_constructor_destructor);
    bool display_members = has_brief_description(cos.functions, function_member);

    std::map<function_type, int> counts;
    BOOST_FOREACH(function const& f, cos.functions)
    {
        counts[f.type]++;
    }

    if (display_ctors && counts[function_constructor_destructor] > 0)
    {
        out << "[heading Constructor(s) and destructor]" << std::endl;
        quickbook_output_function(cos.functions, function_constructor_destructor, config, section_name + ".member", out);
    }

    if (display_members && counts[function_member] > 0)
    {
        out << "[heading Member(s)]" << std::endl;
        quickbook_output_function(cos.functions, function_member, config, section_name + ".member", out);
    }

    // Details start

    //if ( display_ctors || display_members )
    //    out << "[br]" << std::endl;

    if (display_ctors && counts[function_constructor_destructor] > 0)
        quickbook_output_detail_function(cos.functions, function_constructor_destructor, config, "member", out);
    if (display_members && counts[function_member] > 0)
        quickbook_output_detail_function(cos.functions, function_member, config, "member", out);

    // Details end

    out << "[endsect]" << std::endl
        << std::endl;
}

#endif // QUICKBOOK_OUTPUT_HPP
