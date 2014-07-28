#include "precompiled_cpodf.h"

#include "odf_style_context.h"
#include "ods_conversion_context.h"
//#include "odf_text_context.h" ??????

#include "logging.h"
#include "styles.h"

#include "style_paragraph_properties.h"

#include <boost/foreach.hpp>
#include <iostream>

namespace cpdoccore {
namespace odf {

void calc_paragraph_properties_content(std::vector<style_paragraph_properties*> & parProps, paragraph_format_properties * result)
{
	if (result == NULL)return;

    BOOST_REVERSE_FOREACH(style_paragraph_properties* v, parProps)
    {
        if (v)
            result->apply_from(v->content());
    }
}

odf_style_context::odf_style_context()
{        
}

void odf_style_context::set_odf_context(odf_conversion_context * Context)
{
	odf_context_ = Context;
	number_styles_context_.set_odf_context(Context);
	table_styles_context_.set_odf_context(Context);
}

odf_style_state & odf_style_context::last_state()
{
	if (style_state_list_.size() >0)
		return style_state_list_.back();
	else
		throw;
}
odf_style_state * odf_style_context::last_state(style_family family)
{
	for (long i = style_state_list_.size()-1; i>=0; i--)
	{
		if (style_state_list_[i].get_family() == family)
		{
			return &style_state_list_[i];
		}
	}
	return NULL;
}
void odf_style_context::create_style(std::wstring oox_name,const style_family family, bool automatic,bool root,int oox_id)
{
	std::wstring odf_name = oox_name;
	
	if (odf_name.length() <1)odf_name = find_free_name(family);

	office_element_ptr elm;
	create_element(L"style", L"style", elm, odf_context_);
	
	style_state_list_.push_back( odf_style_state(elm, family) ); 

///////////////////////////////////////	
	last_state().set_name(odf_name);
	last_state().set_automatic(automatic);
	last_state().set_root(root);

	last_state().style_oox_id_ = oox_id;
	last_state().style_oox_name_ = oox_name;
}

void odf_style_context::add_style(office_element_ptr elm, bool automatic, bool root, style_family family)
{
	style_state_list_.push_back( odf_style_state(elm, family) ); 
	
	last_state().set_automatic(automatic);
	last_state().set_root(root);
}

void odf_style_context::create_default_style(const style_family family)
{
	office_element_ptr elm;
	create_element(L"style", L"default-style", elm, odf_context_);
	
	style_state_list_.push_back( odf_style_state(elm, family) ); 

///////////////////////////////////////	
	last_state().set_automatic(false);
	last_state().set_root(true);
	last_state().set_default(true);
}
void odf_style_context::reset_defaults()
{
	current_default_styles_.clear();
}

void odf_style_context::process_automatic_for_styles(office_element_ptr root )
{//�������������� ����� ��� ��������� ������
	for (long i =0; i < style_state_list_.size(); i++)
	{
		if (style_state_list_[i].writable_ == false) continue;

		if (style_state_list_[i].automatic_== true && style_state_list_[i].root_== true && style_state_list_[i].odf_style_)
			root->add_child_element(style_state_list_[i].odf_style_);
	}
}
void odf_style_context::process_automatic(office_element_ptr root )
{//�������������� ����� ��� ���������
	for (long i =0; i < style_state_list_.size(); i++)
	{
		if (style_state_list_[i].writable_ == false) continue;

		if (/*it->automatic_== true && */style_state_list_[i].root_== false && style_state_list_[i].odf_style_)
			root->add_child_element(style_state_list_[i].odf_style_);
	}
}

void odf_style_context::process_office(office_element_ptr root )
{
	number_styles_context_.process_styles(root );
	
	for (long i =0; i < style_state_list_.size(); i++)
	{
		if (style_state_list_[i].writable_ == false) continue;

		if (style_state_list_[i].automatic_== false && style_state_list_[i].root_ == true && style_state_list_[i].odf_style_)
			root->add_child_element(style_state_list_[i].odf_style_);
	}
}
std::wstring odf_style_context::find_odf_style_name(int oox_id_style, const style_family family, bool root)
{
	for (long i =0; i < style_state_list_.size(); i++)
	{
		if (style_state_list_[i].odf_style_)
		{
			if (style_state_list_[i].style_family_ == family && 
				style_state_list_[i].root_ == root && style_state_list_[i].conditional_ == false)
			{
				if (oox_id_style >=0 && style_state_list_[i].style_oox_id_ == oox_id_style)	return style_state_list_[i].get_name();
			}
				
		}
	}
	return L"";
}
office_element_ptr odf_style_context::find_odf_style(int oox_id_style, const style_family family, bool root)
{
	//for (std::list<odf_style_state>::iterator it = style_state_list_.begin(); it != style_state_list_.end(); it++)
	for (int i=0;i<style_state_list_.size(); i++)
	{
		if (style_state_list_[i].odf_style_)
		{
			if (style_state_list_[i].style_family_ == family && 
				style_state_list_[i].root_ == root && style_state_list_[i].conditional_ == false)
			{
				if (oox_id_style >=0 && style_state_list_[i].style_oox_id_ == oox_id_style)	return style_state_list_[i].get_office_element();
			}
				
		}
	}
	return office_element_ptr();
}
std::wstring odf_style_context::find_conditional_style_name(int oox_id_style, const style_family family)
{
	for (long i =0; i < style_state_list_.size(); i++)
	{
		if (style_state_list_[i].odf_style_)
		{
			if (style_state_list_[i].style_family_ == family 
						&& style_state_list_[i].conditional_ == true)
			{
				if (oox_id_style >=0 && style_state_list_[i].style_oox_id_ == oox_id_style)	return style_state_list_[i].get_name();
			}
				
		}
	}
	return L"";
}
office_element_ptr odf_style_context::find_conditional_style(int oox_id_style, const style_family family)
{
	for (int i=0;i<style_state_list_.size(); i++)
	{
		if (style_state_list_[i].odf_style_)
		{
			if (style_state_list_[i].style_family_ == family 
						&& style_state_list_[i].conditional_ == true)
			{
				if (oox_id_style >=0 && style_state_list_[i].style_oox_id_ == oox_id_style)	return style_state_list_[i].get_office_element();
			}
				
		}
	}
	return office_element_ptr();
}
bool odf_style_context::find_odf_style_state(int oox_id_style, const style_family family, odf_style_state *& state, bool root)
{
	for (int i=0;i<style_state_list_.size(); i++)
	{
		if (style_state_list_[i].odf_style_)
		{
			if (style_state_list_[i].style_family_ == family && 
				style_state_list_[i].root_ == root && style_state_list_[i].conditional_ == false)
			{
				if (oox_id_style >=0 && style_state_list_[i].style_oox_id_ == oox_id_style)
				{
					state = &style_state_list_[i];
					return true;
				}
			}
				
		}
	}
	return false;
}

bool odf_style_context::find_odf_style(std::wstring style_name, style_family::type family, style *& style_)
{
	for (int i=0;i<style_state_list_.size(); i++)
	{
		if (style_state_list_[i].odf_style_)
		{
			if (style_state_list_[i].style_family_.get_type() == family && style_state_list_[i].get_name() == style_name)
			{
				style_ = dynamic_cast<style*>(style_state_list_[i].odf_style_.get());

				return true;
			}
				
		}
	}
	return false;
}
office_element_ptr odf_style_context::find_odf_style_default(const style_family family)
{
	for (int i=0;i<current_default_styles_.size(); i++)
	{
		if (current_default_styles_[i].odf_style_)
		{
			if (current_default_styles_[i].style_family_ == family)return current_default_styles_[i].get_office_element();
		}
	}
	return office_element_ptr();
}
std::wstring odf_style_context::find_odf_style_name_default(const style_family family)
{
	for (int i=0;i<current_default_styles_.size(); i++)
	{
		if (current_default_styles_[i].odf_style_)
		{
			if (current_default_styles_[i].style_family_ == family)return current_default_styles_[i].get_name();
		}
	}
	return L"";
}
std::wstring odf_style_context::get_name_family(const style_family & family)
{
	switch(family.get_type())
	{
        case style_family::None:		return L"none";
		case style_family::Paragraph:	return L"P";
        case style_family::Text:		return L"T";
        case style_family::Section:		return L"Sect";
        case style_family::Table:		return L"ta";
        case style_family::TableColumn: return L"co";
        case style_family::TableRow:	return L"ro";
        case style_family::TableCell:	return L"ce";
        case style_family::TablePage:	return L"tbl";//??
        case style_family::Chart:		return L"ch";
        case style_family::Default:		return L"def";
        case style_family::DrawingPage: return L"dp";
        case style_family::Graphic:		return L"gr";
        case style_family::Presentation:return L"pr";
        case style_family::Control:		return L"ctrl";//??
        case style_family::Ruby:		return L"rb";//??
 
		case style_family::Gradient:	return L"Gradinent";
        case style_family::Hatch:		return L"Hatch";
		case style_family::Opacity:		return L"Opacity";
		case style_family::FillImage:	return L"FillImage";
		case style_family::Marker:		return L"Marker";
		case style_family::StokeDash:	return L"StokeDash";

		case style_family::PageLayout:	return L"Mpm";
		case style_family::MasterPage:	return L"MasterPage";
		case style_family::HandoutMaster:return L"Handout";
		case style_family::LayerSet:	return L"";//����

	}
	return L"UnknownStyle";
}
std::wstring odf_style_context::find_free_name(const style_family  family)
{
	std::wstring name = get_name_family(family);
	int count =1;

	for (int i=0;i<style_state_list_.size(); i++)
	{
		if ((style_state_list_[i].odf_style_) && (style_state_list_[i].style_family_ == family))
		{
			count++;
		}
	}
	name = name + boost::lexical_cast<std::wstring>(count);
	return name;
}
office_element_ptr & odf_style_context::add_or_find(std::wstring name, const style_family family, bool automatic , bool root, int oox_id)
{
	for (int i=0;i<style_state_list_.size(); i++)
	{
		if (style_state_list_[i].odf_style_)
		{
			if (style_state_list_[i].style_family_ == family)
			{
				if (oox_id >=0 && style_state_list_[i].style_oox_id_ == oox_id)					return style_state_list_[i].get_office_element();
				if ((name.length() >=0 && style_state_list_[i].odf_style_->get_name() == name)
					&& style_state_list_[i].root_ == root)	return style_state_list_[i].get_office_element();
				//�� ��������� ����� ������� ���������� (� �������� ��� root � ��������� - ����� ����������)
			}
				
		}
	}
	create_style(name,family, automatic, root,oox_id);
	
	return last_state().get_office_element();
}

void odf_style_context::calc_paragraph_properties(std::wstring style_name, style_family::type family, paragraph_format_properties * result)
{
    std::vector<style_paragraph_properties*> parProps;
	
	while (style_name.length()>0)
    {
		style *style_ = NULL;
		if (!find_odf_style(style_name, family, style_) || !style_)break;

       if (style_paragraph_properties * parProp = style_->style_content_.get_style_paragraph_properties())
			parProps.push_back(parProp);
        		
	   style_name = style_->style_parent_style_name_ ? *style_->style_parent_style_name_ : L"";

    }   
	calc_paragraph_properties_content(parProps, result);
}


}
}