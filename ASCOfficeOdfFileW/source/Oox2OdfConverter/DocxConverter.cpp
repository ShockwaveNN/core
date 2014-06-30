#pragma once

#include "stdAfx.h"

#include "DocxConverter.h"
#include "../utils.h"

#include <DocxFormat\Docx.h>
#include <DocxFormat\External\Hyperlink.h>
#include <XlsxFormat\Chart\Chart.h>

#include <boost/foreach.hpp>

#include "odf_conversion_context.h"
#include "odt_conversion_context.h"

#include "odf_text_context.h"
#include "odf_drawing_context.h"

#include "styles.h"

#include "style_table_properties.h"
#include "style_text_properties.h"
#include "style_paragraph_properties.h"
#include "style_graphic_properties.h"


using namespace cpdoccore;

namespace Oox2Odf
{
DocxConverter::DocxConverter(const std::wstring & path)
{
	output_document = new  odf::package::odf_document(L"text");

	const OOX::CPath oox_path(CString(path.c_str()));	
	docx_document = new OOX::CDocx(oox_path);	
}
void DocxConverter::write(const std::wstring & path)
{
	if (!output_document)return;

	output_document->write(path);

}
odf::odf_conversion_context* DocxConverter::odf_context()
{
	return odt_context;
}
OOX::CTheme* DocxConverter::oox_theme()
{
	if (docx_document)
		return docx_document->GetTheme();
	else
		return NULL;
}
CString	DocxConverter::find_link_by_id (CString sId, int type)
{
	OOX::CDocument  *oox_doc = docx_document->GetDocument();

	if (oox_doc == NULL)return L"";
		
	CString ref;
	smart_ptr<OOX::File> oFile = docx_document->GetDocument()->Find(sId);

	if (oFile.IsInit())
	{
		if (type==1 && OOX::FileTypes::Image == oFile->type())
		{
			OOX::Image* pImage = (OOX::Image*)oFile.operator->();

			ref = pImage->filename().GetPath();
		}
		if (type == 2 && OOX::FileTypes::HyperLink == oFile->type())
		{
			OOX::HyperLink* pHyperlink = (OOX::HyperLink*)oFile.operator->();

			ref = pHyperlink->Uri().GetPath();
		}
	}
	if (ref.GetLength() < 1 && oox_current_chart)
	{
		smart_ptr<OOX::File> oFile = oox_current_chart->Find(sId);
		if (oFile.IsInit())
		{		
			if (type==1 && OOX::FileTypes::Image == oFile->type())
			{
				OOX::Image* pImage = (OOX::Image*)oFile.operator->();

				ref = pImage->filename().GetPath();
			}
			if (type==2 && oFile.IsInit() && OOX::FileTypes::Image == oFile->type())
			{
				OOX::Image* pImage = (OOX::Image*)oFile.operator->();

				ref = pImage->filename().GetPath();
			}
		}
	}
	return ref;
}

void DocxConverter::convertDocument()
{
	if (!docx_document)return;

	odt_context = new odf::odt_conversion_context(output_document);

	if (!odt_context)return;
		
	odt_context->start_document();

	convert_styles();
	convert_document();

	//������ ��� �������� �������� docx 
	delete docx_document; docx_document = NULL;

	odt_context->end_document();
 
}

void DocxConverter::convert_document()
{
	const OOX::CDocument* document = docx_document->GetDocument();
	if (!document)return;

	for ( int nIndex = 0; nIndex < document->m_arrItems.GetSize(); nIndex++ )
	{
		convert(document->m_arrItems[nIndex]);
	}
}
void DocxConverter::convert(OOX::WritingElement  *oox_unknown)
{
	if (oox_unknown == NULL)return;

	switch(oox_unknown->getType())
	{
		case OOX::et_w_r:
		{
			OOX::Logic::CRun* pRun= static_cast<OOX::Logic::CRun*>(oox_unknown);
			convert(pRun);
		}break;
		case OOX::et_w_p:
		{
			OOX::Logic::CParagraph* pP= static_cast<OOX::Logic::CParagraph*>(oox_unknown);
			convert(pP);
		}break;
		case OOX::et_w_pPr:
		{
			OOX::Logic::CParagraphProperty* pPProp= static_cast<OOX::Logic::CParagraphProperty*>(oox_unknown);
			convert(pPProp);
		}break;
		case OOX::et_w_hyperlink:
		{
			OOX::Logic::CHyperlink* pH= static_cast<OOX::Logic::CHyperlink*>(oox_unknown);
			convert(pH);
		}break;
		case OOX::et_w_rPr:
		{
			OOX::Logic::CRunProperty* pRProp= static_cast<OOX::Logic::CRunProperty*>(oox_unknown);
			convert(pRProp);
		}break;
		case OOX::et_mc_alternateContent:
		{
			OOX::Logic::CAlternateContent* pAltCont= static_cast<OOX::Logic::CAlternateContent*>(oox_unknown);
			convert(pAltCont);
		}break;
		case OOX::et_w_drawing:
		{
			OOX::Logic::CDrawing* pDrawing= static_cast<OOX::Logic::CDrawing*>(oox_unknown);
			convert(pDrawing);
		}break;
		case OOX::et_w_Shape:
		{
			OOX::Logic::CShape* pShape = static_cast<OOX::Logic::CShape*>(oox_unknown);
			convert(pShape);
		}break;
		case OOX::et_w_GroupShape:
		{
			OOX::Logic::CGroupShape* pGroupShape= static_cast<OOX::Logic::CGroupShape*>(oox_unknown);
			convert(pGroupShape);
		}break;
		case OOX::et_w_commentRangeEnd:
		{
			OOX::Logic::CCommentRangeEnd* pCommEnd = static_cast<OOX::Logic::CCommentRangeEnd*>(oox_unknown);
			convert(pCommEnd);
		}break;
		case OOX::et_w_commentRangeStart:
		{
			OOX::Logic::CCommentRangeStart* pCommStart = static_cast<OOX::Logic::CCommentRangeStart*>(oox_unknown);
			convert(pCommStart);
		}break;
		case OOX::et_w_commentReference:
		{
			OOX::Logic::CCommentReference* pCommRef = static_cast<OOX::Logic::CCommentReference*>(oox_unknown);
			convert(pCommRef);		//���� ��� Start - �������� ������ � ����������� Run
		}break;
		default:
		{
			OoxConverter::convert(oox_unknown);
		}break;
	}
}
void DocxConverter::convert(OOX::Logic::CParagraph	*oox_paragraph)
{
	if (oox_paragraph == NULL) return;

	bool styled = false;
	odt_context->start_paragraph();
	
	for ( int nIndex = 0; nIndex < oox_paragraph->m_arrItems.GetSize(); nIndex++ )
	{
		//�� �������� ������� ���� ��� Paragraph - ����� - ��������� � ����� ����		
		switch(oox_paragraph->m_arrItems[nIndex]->getType())
		{
			case OOX::et_w_pPr:
			{
				styled = true;
				OOX::Logic::CParagraphProperty* pPProp= static_cast<OOX::Logic::CParagraphProperty*>(oox_paragraph->m_arrItems[nIndex]);

				odt_context->styles_context()->create_style(L"",odf::style_family::Paragraph, true, false, -1);					
				odf::style_paragraph_properties	* paragraph_properties	= odt_context->styles_context()->last_state().get_paragraph_properties();
				//odf::style_text_properties		* text_properties		= odt_context->styles_context()->last_state().get_text_properties();

				convert(pPProp, paragraph_properties/*, text_properties*/); 
				
				odt_context->text_context()->add_text_style(odt_context->styles_context()->last_state().get_office_element(),odt_context->styles_context()->last_state().get_name());
				odt_context->text_context()->set_single_object(false, paragraph_properties,NULL);// - ���� ��� .. ���� �� ��� ������� ��� ������ �������� - �� �������������
			}break;		
			default:
				convert(oox_paragraph->m_arrItems[nIndex]);
				break;
		}
	}
	odt_context->end_paragraph();
}
void DocxConverter::convert(OOX::Logic::CRun	*oox_run)//wordprocessing 22.1.2.87 math 17.3.2.25
{
	if (oox_run == NULL) return;
	
	odt_context->start_run();
	bool styled = false;
	
	for(int i = 0; i < oox_run->m_arrItems.GetSize(); ++i)
	{
		//�� �������� ������� ���� ��� Run - ����� - ��������� � ����� ����		
		switch(oox_run->m_arrItems[i]->getType())
		{
			case OOX::et_w_fldChar:	
			{
				OOX::Logic::CFldChar* pFldChar= static_cast<OOX::Logic::CFldChar*>(oox_run->m_arrItems[i]);
				convert(pFldChar);
			}break;
			case OOX::et_w_instrText:
			{
				OOX::Logic::CInstrText* pInstrText= static_cast<OOX::Logic::CInstrText*>(oox_run->m_arrItems[i]);
				convert(pInstrText);
			}break;
			case OOX::et_w_rPr:
			{
				styled = true;
				OOX::Logic::CRunProperty* pRProp= static_cast<OOX::Logic::CRunProperty*>(oox_run->m_arrItems[i]);

				odt_context->styles_context()->create_style(L"",odf::style_family::Text, true, false, -1);					
				odf::style_text_properties	* text_properties = odt_context->styles_context()->last_state().get_text_properties();

				convert(pRProp, text_properties);
	
				odf::odf_style_state * state = odt_context->styles_context()->last_state(odf::style_family::Text);
				if (state)
					odt_context->text_context()->add_text_style(state->get_office_element(),state->get_name());
			}break;		
			case OOX::et_w_lastRenderedPageBreak:
			{
				odt_context->text_context()->add_page_break();
			}break;
			case OOX::et_w_br:
			{
				odt_context->text_context()->add_textline_break();
			}break;
			case OOX::et_w_t:
			{
				OOX::Logic::CText* pText= static_cast<OOX::Logic::CText*>(oox_run->m_arrItems[i]);
				convert(pText);
			}break;
			case OOX::et_w_sym:
			{
				OOX::Logic::CSym* pSym= static_cast<OOX::Logic::CSym*>(oox_run->m_arrItems[i]);
				convert(pSym);
			}break;
			case OOX::et_w_tab:
			{
				OOX::Logic::CTab* pTab= static_cast<OOX::Logic::CTab*>(oox_run->m_arrItems[i]);
				odt_context->text_context()->add_tab();
			}break;
			case OOX::et_w_delText:
			{
			}break;
			case OOX::et_w_separator:
			case OOX::et_w_continuationSeparator:
			{
			}break;
			//annotationRef
			//endnoteRef
			//endnoteReference
			//footnoteRef
			//footnoteReference
			//commentReference
			//separator			
			//contentPart
			//cr
			//dayLong, dayShort, monthLong, monthShort, yearLong, yearShort
			//delText
			//noBreakHyphen
			//pgNum
			//ptab
			//ruby
			//softHyphen
			//delInstrText
			default:
				convert(oox_run->m_arrItems[i]);
		}
	}
	if (styled)
	{
		//odf::odf_style_state * state = odt_context->styles_context()->last_state(odf::style_family::Text);
		//if (state)
		//	odt_context->text_context()->add_text_style(state->get_office_element(),state->get_name());
		//�� ����� �������
	}
	odt_context->end_run();

}
void DocxConverter::convert(OOX::Logic::CFldChar	*oox_fld)
{
	if (oox_fld == NULL) return;

		//nullable<SimpleTypes::COnOff<>       > m_oDirty;
		//nullable<SimpleTypes::COnOff<>       > m_oFldLock;
		//nullable<OOX::Logic::CFFData         > m_oFFData;
	if (oox_fld->m_oFldCharType.IsInit())
	{
		if (oox_fld->m_oFldCharType->GetValue() == SimpleTypes::fldchartypeBegin)	odt_context->start_field();
		if (oox_fld->m_oFldCharType->GetValue() == SimpleTypes::fldchartypeEnd)		odt_context->end_field();
	}

}
void DocxConverter::convert(OOX::Logic::CInstrText	*oox_instr)
{
	if (oox_instr == NULL) return;

	odt_context->set_field_instr(string2std_string(oox_instr->m_sText));

}
void DocxConverter::convert(OOX::Logic::CParagraphProperty	*oox_paragraph_pr, cpdoccore::odf::style_paragraph_properties * paragraph_properties)
{
	if (!oox_paragraph_pr)		return;
	if (!paragraph_properties)	return;

	if (oox_paragraph_pr->m_oPStyle.IsInit() && oox_paragraph_pr->m_oPStyle->m_sVal.IsInit())
	{
		odt_context->styles_context()->last_state().set_parent_style_name(string2std_string(*oox_paragraph_pr->m_oPStyle->m_sVal));
	}

	if (oox_paragraph_pr->m_oSpacing.IsInit())
	{
		SimpleTypes::ELineSpacingRule rule = SimpleTypes::linespacingruleAtLeast;
		if (oox_paragraph_pr->m_oSpacing->m_oLineRule.IsInit())rule = oox_paragraph_pr->m_oSpacing->m_oLineRule->GetValue();

		if (oox_paragraph_pr->m_oSpacing->m_oLine.IsInit())
		{
			_CP_OPT(odf::length) length;
			convert(static_cast<SimpleTypes::CUniversalMeasure *>(oox_paragraph_pr->m_oSpacing->m_oLine.GetPointer()), length);
			if (length && rule == SimpleTypes::linespacingruleExact)
				paragraph_properties->content().fo_line_height_ = odf::line_width(*length);
			else if (length)
				paragraph_properties->content().style_line_height_at_least_= length;
		}
		if (oox_paragraph_pr->m_oSpacing->m_oAfter.IsInit())
		{
 			_CP_OPT(odf::length_or_percent) length;
			convert(static_cast<SimpleTypes::CUniversalMeasure *>(oox_paragraph_pr->m_oSpacing->m_oAfter.GetPointer()), length);
			paragraph_properties->content().fo_margin_bottom_ = length;
		}
		if (oox_paragraph_pr->m_oSpacing->m_oBefore.IsInit())
		{
 			_CP_OPT(odf::length_or_percent) length;
			convert(static_cast<SimpleTypes::CUniversalMeasure *>(oox_paragraph_pr->m_oSpacing->m_oBefore.GetPointer()), length);
			paragraph_properties->content().fo_margin_top_ = length;
		}
			//nullable<SimpleTypes::COnOff<>            > m_oAfterAutospacing;
			//nullable<SimpleTypes::CDecimalNumber<>    > m_oAfterLines;
			//nullable<SimpleTypes::COnOff<>            > m_oBeforeAutospacing;
			//nullable<SimpleTypes::CDecimalNumber<>    > m_oBeforeLines;
	}
	if (oox_paragraph_pr->m_oContextualSpacing.IsInit())
		paragraph_properties->content().style_contextual_spacing_ = true;

	if (oox_paragraph_pr->m_oInd.IsInit())
	{
		if (oox_paragraph_pr->m_oInd->m_oStart.IsInit())
		{
 			_CP_OPT(odf::length_or_percent) length;
			convert(static_cast<SimpleTypes::CUniversalMeasure *>(oox_paragraph_pr->m_oInd->m_oStart.GetPointer()), length);
			paragraph_properties->content().fo_margin_left_ = length;
		}
		if (oox_paragraph_pr->m_oInd->m_oEnd.IsInit())
		{
 			_CP_OPT(odf::length_or_percent) length;
			convert(static_cast<SimpleTypes::CUniversalMeasure *>(oox_paragraph_pr->m_oInd->m_oEnd.GetPointer()), length);
			paragraph_properties->content().fo_margin_right_ = length;
		}
		if (oox_paragraph_pr->m_oInd->m_oFirstLine.IsInit())
		{
 			_CP_OPT(odf::length_or_percent) length;
			convert(static_cast<SimpleTypes::CUniversalMeasure *>(oox_paragraph_pr->m_oInd->m_oEnd.GetPointer()), length);
			paragraph_properties->content().fo_text_indent_ = length;
		}
		if (oox_paragraph_pr->m_oInd->m_oHanging.IsInit())
		{
		}
			//nullable<SimpleTypes::CDecimalNumber<>    > m_oEndChars;
			//nullable<SimpleTypes::CDecimalNumber<>    > m_oFirstLineChars;
			//nullable<SimpleTypes::CDecimalNumber<>    > m_oHangingChars;
			//nullable<SimpleTypes::CDecimalNumber<>    > m_oStartChars;
	}
	//if (oox_paragraph_pr->m_oRtl.IsInit())
	//{
	//}	
	convert(oox_paragraph_pr->m_oJc.GetPointer(), paragraph_properties->content().fo_text_align_);

	if (oox_paragraph_pr->m_oTextAlignment.IsInit() && oox_paragraph_pr->m_oTextAlignment->m_oVal.IsInit())
	{
		switch(oox_paragraph_pr->m_oTextAlignment->m_oVal->GetValue())
		{
		//case SimpleTypes::textalignAuto     :
		//case SimpleTypes::textalignBaseLine :
		//case SimpleTypes::textalignBottom   :
		//case SimpleTypes::textalignCenter   :
		//case SimpleTypes::textalignTop      :
		}
	}

	//if (oox_paragraph_pr->m_oWordWrap.IsInit())	odt_context->set_word_wrap(oox_paragraph_pr->m_oWordWrap->ToBool());

	convert(oox_paragraph_pr->m_oPBdr.GetPointer(), paragraph_properties);

	if (oox_paragraph_pr->m_oRPr.IsInit())//����� ���� ������ !!!
	{
		odf::style_text_properties * text_properties = odf_context()->text_context()->get_text_properties();
		if (text_properties) 
			convert(oox_paragraph_pr->m_oRPr.GetPointer(), text_properties);
	}
	if (oox_paragraph_pr->m_oShd.IsInit())
	{
		_CP_OPT(odf::color) odf_color;
		convert(oox_paragraph_pr->m_oShd->m_oFill.GetPointer(), oox_paragraph_pr->m_oShd->m_oThemeFill.GetPointer(),
			oox_paragraph_pr->m_oShd->m_oThemeFillTint.GetPointer(), oox_paragraph_pr->m_oShd->m_oThemeShade.GetPointer(), odf_color);
		if (odf_color)
			paragraph_properties->content().fo_background_color_ = *odf_color;
	}
	if (oox_paragraph_pr->m_oTextDirection.IsInit() && oox_paragraph_pr->m_oTextDirection->m_oVal.IsInit())
	{
		switch(oox_paragraph_pr->m_oTextDirection->m_oVal->GetValue())
		{
		case SimpleTypes::textdirectionLr   :
			paragraph_properties->content().style_writing_mode_= odf::writing_mode(odf::writing_mode::Lr); break;
		case SimpleTypes::textdirectionLrV  :
			paragraph_properties->content().style_writing_mode_= odf::writing_mode(odf::writing_mode::LrTb); break;
		case SimpleTypes::textdirectionRl   :
			paragraph_properties->content().style_writing_mode_= odf::writing_mode(odf::writing_mode::Rl); break;
		case SimpleTypes::textdirectionRlV  :
			paragraph_properties->content().style_writing_mode_= odf::writing_mode(odf::writing_mode::RlTb); break;
		case SimpleTypes::textdirectionTb   :
			paragraph_properties->content().style_writing_mode_= odf::writing_mode(odf::writing_mode::Tb); break;
		case SimpleTypes::textdirectionTbV  :
			paragraph_properties->content().style_writing_mode_= odf::writing_mode(odf::writing_mode::TbLr); break;
		}
	}
	//m_oSectPr

}
void DocxConverter::convert(OOX::Logic::CPBdr *oox_border, odf::style_paragraph_properties * paragraph_properties)
{
	if (oox_border == NULL) return;
	if (paragraph_properties == NULL) return;

	std::wstring odf_border;
	if (oox_border->m_oBottom.IsInit())
	{
		convert(oox_border->m_oBottom.GetPointer(), odf_border);
		if (paragraph_properties && odf_border.length() >0 ) paragraph_properties->content().fo_border_bottom_ = odf_border;
	}
	if (oox_border->m_oTop.IsInit())
	{
		convert(oox_border->m_oTop.GetPointer(), odf_border);
		if (paragraph_properties && odf_border.length() >0 ) paragraph_properties->content().fo_border_top_ = odf_border;
	}
	if (oox_border->m_oLeft.IsInit())
	{
		convert(oox_border->m_oLeft.GetPointer(), odf_border);
		if (paragraph_properties && odf_border.length() >0 ) paragraph_properties->content().fo_border_left_ = odf_border;
	}
	if (oox_border->m_oRight.IsInit())
	{
		convert(oox_border->m_oRight.GetPointer(), odf_border);
		if (paragraph_properties && odf_border.length() >0 ) paragraph_properties->content().fo_border_right_ = odf_border;
	}
	//if (oox_border->m_oTL2BR.IsInit())
	//{
	//	convert(oox_border->m_oTL2BR.GetPointer(), odf_border);
	//	if (paragraph_properties && odf_border.length() >0 ) paragraph_properties->content().style_diagonal_tl_br_ = odf_border;
	//}

}
void DocxConverter::convert(ComplexTypes::Word::CBorder *borderProp, std::wstring & odf_border_prop)
{
	if (!borderProp)return;

	std::wstring border_style;
	std::wstring border_color;
			//nullable<SimpleTypes::COnOff<>              > m_oFrame;
			//nullable<SimpleTypes::COnOff<>              > m_oShadow;
			//nullable<SimpleTypes::CPointMeasure<>       > m_oSpace;
			//nullable<SimpleTypes::CUcharHexNumber<>     > m_oThemeShade;

	if (borderProp->m_oSz.IsInit()) border_style = boost::lexical_cast<std::wstring>(borderProp->m_oSz->ToPoints()) + L"pt";
	else border_style = L"1pt";
	if (borderProp->m_oVal.IsInit())//266 styles  oO
	{
		switch(borderProp->m_oVal->GetValue())
		{
			case SimpleTypes::bordervalueDotDash:
				border_style += L" dashed";
			break;
			case SimpleTypes::bordervalueDashed:
				border_style += L" dashed";
			break;
			case SimpleTypes::bordervalueDotted:
				border_style += L" dotted";
			break;
			case SimpleTypes::bordervalueDouble:
				border_style += L" double";
			break;
			case SimpleTypes::bordervalueSingle:
				border_style += L" solid";
			break;
			case SimpleTypes::bordervalueNone:
				border_style == L" none";
				return;
			break;
		}
	}else border_style += L" solid";
///////////////////////////////////////////////////////////////////
	_CP_OPT(odf::color) color;

	convert(borderProp->m_oColor.GetPointer(), borderProp->m_oThemeColor.GetPointer(), borderProp->m_oThemeTint.GetPointer(),borderProp->m_oThemeShade.GetPointer(), color);

	if (color) border_color = color->get_hex_value();
	else border_color = L"000000";
	
	odf_border_prop = border_style + L" #" + border_color;
}
void DocxConverter::convert(ComplexTypes::Word::CJc * oox_jc,  _CP_OPT(odf::text_align) & align)
{
	if (oox_jc == NULL) return;
	if (oox_jc->m_oVal.IsInit() == false) return;

	switch(oox_jc->m_oVal->GetValue())
	{
		case SimpleTypes::jcBoth            : align = odf::text_align(odf::text_align::Justify);break;
		case SimpleTypes::jcCenter          : align = odf::text_align(odf::text_align::Center);	break;
		case SimpleTypes::jcThaiDistribute  :	
		case SimpleTypes::jcDistribute      : align = odf::text_align(odf::text_align::Justify);break;
		case SimpleTypes::jcEnd             : align = odf::text_align(odf::text_align::End);	break;
		case SimpleTypes::jcHighKashida     :	break;
		case SimpleTypes::jcLowKashida      :	break;
		case SimpleTypes::jcMediumKashida   :	break;
		case SimpleTypes::jcNumTab          :	break;
		case SimpleTypes::jcStart           : align = odf::text_align(odf::text_align::Start);	break;
		case SimpleTypes::jcLeft            : align = odf::text_align(odf::text_align::Left);	break;
		case SimpleTypes::jcRight           : align = odf::text_align(odf::text_align::Right);	break;
	}
}
void DocxConverter::convert(SimpleTypes::CUniversalMeasure *oox_size, _CP_OPT(odf::length) & odf_size)
{
	if (oox_size == NULL)return;

	odf_size = odf::length(oox_size->ToPoints(), odf::length::pt);
}
void DocxConverter::convert(SimpleTypes::CUniversalMeasure* oox_size, _CP_OPT(odf::length_or_percent) & odf_size)
{
	if (oox_size == NULL)return;

		//odf_size = odf::percent(oox_size->ToPoints(());
	odf_size = odf::length(oox_size->ToPoints(), odf::length::pt);

}

void DocxConverter::convert(OOX::Logic::CRunProperty *oox_run_pr, odf::style_text_properties * text_properties)
{
	if (oox_run_pr == NULL) return;
	if (text_properties == NULL)return;
	
	if (oox_run_pr->m_oRStyle.IsInit() && oox_run_pr->m_oRStyle->m_sVal.IsInit())
	{
		odt_context->styles_context()->last_state().set_parent_style_name(string2std_string(*oox_run_pr->m_oRStyle->m_sVal));
	}
	if (oox_run_pr->m_oBold.IsInit())
	{
		if (oox_run_pr->m_oBold->m_oVal.ToBool() ==true) 
			text_properties->content().fo_font_weight_ = odf::font_weight(odf::font_weight::WBold);
		else
			text_properties->content().fo_font_weight_ = odf::font_weight(odf::font_weight::WNormal);
	}
	convert(oox_run_pr->m_oColor.GetPointer(),text_properties->content().fo_color_);

	text_properties->content().style_text_underline_type_= odf::line_type(odf::line_type::None);
	if (oox_run_pr->m_oU.IsInit())
	{
		text_properties->content().style_text_underline_style_ = odf::line_style(odf::line_style::Solid);
		text_properties->content().style_text_underline_type_= odf::line_type(odf::line_type::Single);
		if (oox_run_pr->m_oU->m_oVal.IsInit())
		{
			SimpleTypes::EUnderline 	type = oox_run_pr->m_oU->m_oVal->GetValue();
			text_properties->content().style_text_underline_type_= odf::line_type(odf::line_type::Single);
			switch(type)
			{
			case SimpleTypes::underlineDouble		:
					text_properties->content().style_text_underline_type_= odf::line_type(odf::line_type::Double);break;
			case SimpleTypes::underlineNone			:
					text_properties->content().style_text_underline_type_= odf::line_type(odf::line_type::None);break;
			case SimpleTypes::underlineDash            :
			case SimpleTypes::underlineDashedHeavy     :
					text_properties->content().style_text_underline_style_= odf::line_style(odf::line_style::Dash);break;
			case SimpleTypes::underlineDashLong        :
			case SimpleTypes::underlineDashLongHeavy   :
					text_properties->content().style_text_underline_style_= odf::line_style(odf::line_style::LongDash);break;
			case SimpleTypes::underlineDotDash         :
			case SimpleTypes::underlineDashDotHeavy    :
					text_properties->content().style_text_underline_style_= odf::line_style(odf::line_style::DotDash);break;
			case SimpleTypes::underlineDotted          :
			case SimpleTypes::underlineDottedHeavy     :
					text_properties->content().style_text_underline_style_= odf::line_style(odf::line_style::Dotted);break;
			case SimpleTypes::underlineDotDotDash      :
			case SimpleTypes::underlineDashDotDotHeavy :
					text_properties->content().style_text_underline_style_= odf::line_style(odf::line_style::DotDotDash);break;
			case SimpleTypes::underlineThick           :
			case SimpleTypes::underlineWave            :
			case SimpleTypes::underlineWavyDouble      :
			case SimpleTypes::underlineWavyHeavy       :
					text_properties->content().style_text_underline_style_= odf::line_style(odf::line_style::Wave);break;
			}
		}
		text_properties->content().style_text_underline_color_ = odf::underline_color(odf::underline_color::FontColor);
	}
	if (oox_run_pr->m_oItalic.IsInit())
	{
		if (oox_run_pr->m_oItalic->m_oVal.ToBool() ==true)
			text_properties->content().fo_font_style_ = odf::font_style(odf::font_style::Italic);
		else
			text_properties->content().fo_font_style_ = odf::font_style(odf::font_style::Normal);
	}
	if (oox_run_pr->m_oSz.IsInit() && oox_run_pr->m_oSz->m_oVal.IsInit())
	{
		OoxConverter::convert(oox_run_pr->m_oSz->m_oVal->ToPoints(), text_properties->content().fo_font_size_);
	}
	if (oox_run_pr->m_oCaps.IsInit() && oox_run_pr->m_oCaps->m_oVal.ToBool())
		text_properties->content().fo_text_transform_ = odf::text_transform(odf::text_transform::Uppercase);
	
	if (oox_run_pr->m_oSmallCaps.IsInit() && oox_run_pr->m_oSmallCaps->m_oVal.ToBool())
		text_properties->content().fo_font_variant_ = odf::font_variant(odf::font_variant::SmallCaps);

	if (oox_run_pr->m_oRFonts.IsInit())
	{
		if (oox_run_pr->m_oRFonts->m_sAscii.IsInit())
			text_properties->content().fo_font_family_ = string2std_string(oox_run_pr->m_oRFonts->m_sAscii.get());
		if (oox_run_pr->m_oRFonts->m_sCs.IsInit())
			text_properties->content().style_font_family_complex_ = string2std_string(oox_run_pr->m_oRFonts->m_sCs.get());
		if (oox_run_pr->m_oRFonts->m_sEastAsia.IsInit())
			text_properties->content().style_font_family_asian_= string2std_string(oox_run_pr->m_oRFonts->m_sEastAsia.get());
			//nullable<CString              > m_sAscii;
			//nullable<SimpleTypes::CTheme<>> m_oAsciiTheme;
			//nullable<SimpleTypes::CTheme<>> m_oCsTheme;
			//nullable<CString              > m_sEastAsia;
			//nullable<SimpleTypes::CTheme<>> m_oEastAsiaTheme;
			//nullable<CString              > m_sHAnsi;
			//nullable<SimpleTypes::CTheme<>> m_oHAnsiTheme;
			//nullable<SimpleTypes::CHint<> > m_oHint;
	}
	if (oox_run_pr->m_oVertAlign.IsInit() && oox_run_pr->m_oVertAlign->m_oVal.IsInit())
	{ 
		switch(oox_run_pr->m_oVertAlign->m_oVal->GetValue())
		{
		case SimpleTypes::verticalalignrunSuperscript:
			//text_properties->content().style_text_position_ = odf::text_position(odf::text_position::Super); break;
			text_properties->content().style_text_position_ = odf::text_position(+33.); break;
		case SimpleTypes::verticalalignrunSubscript:
			//text_properties->content().style_text_position_ = odf::text_position(odf::text_position::Sub); break;
			text_properties->content().style_text_position_ = odf::text_position(-33.); break;//�� ��������� 58% - ���� ��������
		}
	}
	if (oox_run_pr->m_oW.IsInit() && oox_run_pr->m_oW->m_oVal.IsInit())
	{
			text_properties->content().style_text_scale_ = odf::percent(oox_run_pr->m_oW->m_oVal->GetValue());
	}
	if (oox_run_pr->m_oStrike.IsInit())
		text_properties->content().style_text_line_through_type_ = odf::line_type(odf::line_type::Single);

	if (oox_run_pr->m_oBdr.IsInit())
	{
		std::wstring odf_border;
		convert(oox_run_pr->m_oBdr.GetPointer(), odf_border);
		if (odf_border.length() > 0)
			text_properties->content().common_border_attlist_.fo_border_ = odf_border;
	}
	if (oox_run_pr->m_oHighlight.IsInit() && oox_run_pr->m_oHighlight->m_oVal.IsInit())
	{
		BYTE ucR = oox_run_pr->m_oHighlight->m_oVal->Get_R(); 
		BYTE ucB = oox_run_pr->m_oHighlight->m_oVal->Get_B(); 
		BYTE ucG = oox_run_pr->m_oHighlight->m_oVal->Get_G(); 
		SimpleTypes::CHexColor<> *oRgbColor = new SimpleTypes::CHexColor<>(ucR,ucG,ucB);
		if (oRgbColor)
		{		
			text_properties->content().fo_background_color_ = odf::color(std::wstring(L"#") + string2std_string(oRgbColor->ToString().Right(6)));
			delete oRgbColor;
		}
	}
	if (oox_run_pr->m_oShd.IsInit())
	{
		_CP_OPT(odf::color) odf_color;
		convert(oox_run_pr->m_oShd->m_oFill.GetPointer(), oox_run_pr->m_oShd->m_oThemeFill.GetPointer(),
			oox_run_pr->m_oShd->m_oThemeFillTint.GetPointer(), oox_run_pr->m_oShd->m_oThemeShade.GetPointer(), odf_color);
		if (odf_color)
			text_properties->content().fo_background_color_ = *odf_color;
	}
}

void DocxConverter::convert(OOX::Logic::CText *oox_text)
{
	if (oox_text == NULL) return;

	odt_context->add_text_content(string2std_string(oox_text->m_sText));
}
void DocxConverter::convert(OOX::Logic::CAlternateContent *oox_alt_content)
{
	if (oox_alt_content == NULL)return;

	for(int i = 0; i < oox_alt_content->m_arrChoiceItems.GetSize(); ++i)
	{
		convert(oox_alt_content->m_arrChoiceItems[i]);
	}
	for(int i = 0; i < oox_alt_content->m_arrFallbackItems.GetSize(); ++i)
	{
		convert(oox_alt_content->m_arrFallbackItems[i]);
	}
}
void DocxConverter::convert(OOX::Logic::CDrawing *oox_drawing)
{
	if (oox_drawing == NULL) return;

	odt_context->start_drawings();
		convert(oox_drawing->m_oAnchor.GetPointer());
		convert(oox_drawing->m_oInline.GetPointer());
	odt_context->end_drawings();
}
void DocxConverter::convert(OOX::Drawing::CAnchor *oox_anchor)
{
	if (oox_anchor == NULL)return;

	double width =0, height=0;
	double x =0, y=0;
	if (oox_anchor->m_oExtent.IsInit()) //size
	{
		width = oox_anchor->m_oExtent->m_oCx.ToPoints();
		height = oox_anchor->m_oExtent->m_oCx.ToPoints();
	}
	if (oox_anchor->m_oDistL.IsInit())odt_context->drawing_context()->set_margin_left(oox_anchor->m_oDistL->ToPoints());
	if (oox_anchor->m_oDistT.IsInit())odt_context->drawing_context()->set_margin_top(oox_anchor->m_oDistT->ToPoints());
	if (oox_anchor->m_oDistR.IsInit())odt_context->drawing_context()->set_margin_right(oox_anchor->m_oDistR->ToPoints());
	if (oox_anchor->m_oDistB.IsInit())odt_context->drawing_context()->set_margin_bottom(oox_anchor->m_oDistB->ToPoints());

	odt_context->drawing_context()->set_drawings_rect(-1, -1, width, height);

	if (oox_anchor->m_oPositionV.IsInit() && oox_anchor->m_oPositionV->m_oRelativeFrom.IsInit())
	{
		odt_context->drawing_context()->set_vertical_rel(oox_anchor->m_oPositionV->m_oRelativeFrom->GetValue());

		if ( oox_anchor->m_oPositionV->m_oAlign.IsInit())
			odt_context->drawing_context()->set_vertical_pos(oox_anchor->m_oPositionV->m_oAlign->GetValue());

		else if(oox_anchor->m_oPositionV->m_oPosOffset.IsInit())
			odt_context->drawing_context()->set_vertical_pos(oox_anchor->m_oPositionV->m_oPosOffset->ToPoints());
		else
			odt_context->drawing_context()->set_vertical_pos(SimpleTypes::alignvTop);
	}
	if (oox_anchor->m_oPositionH.IsInit() && oox_anchor->m_oPositionH->m_oRelativeFrom.IsInit())
	{
		odt_context->drawing_context()->set_horizontal_rel(oox_anchor->m_oPositionH->m_oRelativeFrom->GetValue());
		
		if (oox_anchor->m_oPositionH->m_oAlign.IsInit())
			odt_context->drawing_context()->set_horizontal_pos(oox_anchor->m_oPositionH->m_oAlign->GetValue());
		
		else if(oox_anchor->m_oPositionH->m_oPosOffset.IsInit())
			odt_context->drawing_context()->set_horizontal_pos(oox_anchor->m_oPositionH->m_oPosOffset->ToPoints());
		else
			odt_context->drawing_context()->set_horizontal_pos(SimpleTypes::alignhLeft);
	}
	//if (oox_anchor->m_oWrapNone.IsInit())
	//{
	//}
	if (oox_anchor->m_oWrapSquare.IsInit())
	{
		if (oox_anchor->m_oWrapSquare->m_oWrapText.IsInit() && oox_anchor->m_oWrapSquare->m_oWrapText->GetValue() == SimpleTypes::wraptextLargest)
			odt_context->drawing_context()->set_wrap_style(odf::style_wrap::Dynamic);
		else	
			odt_context->drawing_context()->set_wrap_style(odf::style_wrap::Parallel);
	}	
	if (oox_anchor->m_oWrapThrough.IsInit())//style:wrap="run-through" draw:wrap-influence-on-position style:wrap-contour
	{
		odt_context->drawing_context()->set_wrap_style(odf::style_wrap::RunThrough);
	}
	if (oox_anchor->m_oWrapTight.IsInit())
	{
		odt_context->drawing_context()->set_wrap_style(odf::style_wrap::Parallel);
	}
	if (oox_anchor->m_oWrapTopAndBottom.IsInit())
	{
		odt_context->drawing_context()->set_wrap_style(odf::style_wrap::Parallel);
	}
	convert(oox_anchor->m_oGraphic.GetPointer());
}
void DocxConverter::convert(OOX::Drawing::CInline *oox_inline)
{
	if (oox_inline == NULL)return;

	double width =0, height=0;
	if (oox_inline->m_oExtent.IsInit()) //size
	{
		width = oox_inline->m_oExtent->m_oCx.ToPoints();
		height = oox_inline->m_oExtent->m_oCy.ToPoints();
	}

	odt_context->drawing_context()->set_drawings_rect(0, 0, width, height);

	odt_context->drawing_context()->set_anchor(odf::anchor_type::AsChar); 
	
	if (oox_inline->m_oDistL.IsInit())odt_context->drawing_context()->set_margin_left(oox_inline->m_oDistL->ToPoints());
	if (oox_inline->m_oDistT.IsInit())odt_context->drawing_context()->set_margin_top(oox_inline->m_oDistT->ToPoints());
	if (oox_inline->m_oDistR.IsInit())odt_context->drawing_context()->set_margin_right(oox_inline->m_oDistR->ToPoints());
	if (oox_inline->m_oDistB.IsInit())odt_context->drawing_context()->set_margin_bottom(oox_inline->m_oDistB->ToPoints());

	convert(oox_inline->m_oGraphic.GetPointer());
}
void DocxConverter::convert(OOX::Drawing::CGraphic *oox_graphic)
{
	if (oox_graphic == NULL)return;

	convert(oox_graphic->m_oPicture.GetPointer());
	convert(oox_graphic->m_oChart.GetPointer());
	convert(oox_graphic->m_oShape.GetPointer());
	convert(oox_graphic->m_oGroupShape.GetPointer());
}

void DocxConverter::convert(OOX::Drawing::CPicture * oox_picture)
{
	if (!oox_picture)return;

	odt_context->drawing_context()->start_drawing();

	CString pathImage;
	double Width=0, Height = 0;

	if (oox_picture->m_oBlipFill.m_oBlip.IsInit())
	{
		CString sID = oox_picture->m_oBlipFill.m_oBlip->m_oEmbed.GetValue();		
		pathImage = find_link_by_id(sID,1);
		
		if (pathImage.GetLength() < 1)
		{
			sID = oox_picture->m_oBlipFill.m_oBlip->m_oLink.GetValue();	
			//???
		}
		_gdi_graphics_::GetResolution(pathImage, Width, Height);
	}
	odt_context->start_image(string2std_string(pathImage));
	{
		if (oox_picture->m_oBlipFill.m_oTile.IsInit()) 
		{
			odt_context->drawing_context()->set_image_style_repeat(2);
		}
		if (oox_picture->m_oBlipFill.m_oStretch.IsInit())
		{
			odt_context->drawing_context()->set_image_style_repeat(1);
		}
		if (oox_picture->m_oBlipFill.m_oSrcRect.IsInit() && Width >0 && Height >0)
		{
			odt_context->drawing_context()->set_image_client_rect(oox_picture->m_oBlipFill.m_oSrcRect->m_oL.GetValue() * Width/100. ,
																 oox_picture->m_oBlipFill.m_oSrcRect->m_oT.GetValue() * Height/100.,
																 oox_picture->m_oBlipFill.m_oSrcRect->m_oR.GetValue() * Width/100. , 
																 oox_picture->m_oBlipFill.m_oSrcRect->m_oB.GetValue() * Height/100.);
		}		

		OoxConverter::convert(&oox_picture->m_oNvPicPr.m_oCNvPr);		

		//oox_picture->m_oNvPicPr.m_oCNvPicPr
		//oox_picture->m_oNvPicPr.m_oCNvPicPr.m_oPicLocks
		{
			//if (oox_picture->m_oNvPicPr.m_oCNvPicPr.m_oPicLocks->m_oNoChangeAspect)
			//{
			//}
			//if (oox_picture->m_oNvPicPr.m_oCNvPicPr.m_oPicLocks->m_oNoCrop))
			//{
			//}
			//if (oox_picture->m_oNvPicPr.m_oCNvPicPr.m_oPicLocks->m_oNoResize)
			//{
			//}
		}	
		//m_oExtLst


		OoxConverter::convert(&oox_picture->m_oSpPr, NULL);

	}
	odt_context->drawing_context()->end_image();
	odt_context->drawing_context()->end_drawing();
}

void DocxConverter::convert(OOX::Drawing::CChart * oox_chart)
{
	if (oox_chart == NULL)return;

	if (oox_chart->m_oRId.IsInit())
	{
		smart_ptr<OOX::File> oFile = docx_document->GetDocument()->Find(oox_chart->m_oRId->GetValue());
		if (oFile.IsInit() && OOX::FileTypes::Chart == oFile->type())
		{
			OOX::Spreadsheet::CChartSpace* pChart = (OOX::Spreadsheet::CChartSpace*)oFile.operator->();
			
			if (pChart)
			{
				odt_context->drawing_context()->start_drawing();				
				odt_context->drawing_context()->start_object(odf_context()->get_next_name_object());

					double width =0, height =0;
					odt_context->drawing_context()->get_size(width, height);

					OoxConverter::convert(pChart->m_oChartSpace.m_oSpPr.GetPointer());
					
					oox_current_chart = pChart;
					odf_context()->start_chart();
						odf_context()->chart_context()->set_chart_size(width, height);
						OoxConverter::convert(&pChart->m_oChartSpace);
					odf_context()->end_chart();
					oox_current_chart = NULL; // object???

				odt_context->drawing_context()->end_object();	
				odt_context->drawing_context()->end_drawing();
			}
		}
	}

}

void DocxConverter::convert(OOX::Logic::CGroupShape	 *oox_group_shape)
{
	if (oox_group_shape == NULL)return;
	if (oox_group_shape->m_arrItems.GetSize() < 1) return;

	std::wstring name;
	int id = -1;
	
	//if (oox_group_shape->m_oCNvPr.IsInit())
	{
		//if (oox_group_shape->m_oCNvPr->m_sName.IsInit())
		//		name = string2std_string(*oox_group_shape->m_oCNvPr->m_sName);
		//if (oox_group_shape->m_oCNvPr->m_oId.IsInit())
		//		id = oox_group_shape->m_oCNvPr->m_oId->GetValue();
	}

	odt_context->drawing_context()->start_group(name,id);

	OoxConverter::convert(oox_group_shape->m_oGroupSpPr.GetPointer());

	for (long i=0; i < oox_group_shape->m_arrItems.GetSize(); i++)
	{
		convert(oox_group_shape->m_arrItems[i]);
	}

	odt_context->drawing_context()->end_group();
}
void DocxConverter::convert(OOX::Logic::CShape	 *oox_shape)
{
	if (oox_shape == NULL)return;
	if (!oox_shape->m_oSpPr.IsInit()) return;

	odt_context->drawing_context()->start_drawing();
	
		int type = -1;
		if (oox_shape->m_oSpPr->m_oCustGeom.IsInit())
		{
			type = 1000;//6???
		}
		if (oox_shape->m_oSpPr->m_oPrstGeom.IsInit())
		{
			OOX::Drawing::CPresetGeometry2D * geometry = oox_shape->m_oSpPr->m_oPrstGeom.GetPointer();
			type =(geometry->m_oPrst.GetValue());
		}
		if (oox_shape->m_oCNvSpPr.IsInit())
		{
			if (oox_shape->m_oCNvSpPr->m_otxBox.GetValue() == 1)
				type = 2000; //textBox
		}
		if (type < 0)return;
	/////////////////////////////////////////////////////////////////////////////////
		if (type == 2000)	odt_context->drawing_context()->start_text_box(); 
		else				odt_context->drawing_context()->start_shape(type);
		
		OoxConverter::convert(oox_shape->m_oSpPr.GetPointer(), oox_shape->m_oShapeStyle.GetPointer());
	//���, ���������, ����� ...		
		OoxConverter::convert(oox_shape->m_oCNvPr.GetPointer());	
	//����������������� 
		OoxConverter::convert(oox_shape->m_oCNvSpPr.GetPointer());	
		
		if (oox_shape->m_oCNvConnSpPr.IsInit())
		{
			OoxConverter::convert(oox_shape->m_oCNvConnSpPr.GetPointer());
			//��������� ���������� �� ����� ����������� ������� .. � �� �� ���������� :)
			odf_context()->drawing_context()->start_area_properies();
				odf_context()->drawing_context()->set_no_fill();
			odf_context()->drawing_context()->end_area_properies();
		}
		if (oox_shape->m_oTxBody.IsInit() && oox_shape->m_oTxBody->m_oTxtbxContent.IsInit())
		{
			odt_context->start_text_context();
			for (long i=0 ; i < oox_shape->m_oTxBody->m_oTxtbxContent->m_arrItems.GetSize();i++)
			{
				convert(oox_shape->m_oTxBody->m_oTxtbxContent->m_arrItems[i]);
			}
			odt_context->drawing_context()->set_text( odt_context->text_context());
			odt_context->end_text_context();	
		}

	if (type == 2000)	odt_context->drawing_context()->end_text_box();
	else				odt_context->drawing_context()->end_shape();
	
	odt_context->drawing_context()->end_drawing();
}

void DocxConverter::convert(SimpleTypes::CHexColor<>		*color, 
							SimpleTypes::CThemeColor<>		*theme_color, 
							SimpleTypes::CUcharHexNumber<>	*theme_tint,
							SimpleTypes::CUcharHexNumber<>	*theme_shade, _CP_OPT(odf::color) & odf_color)
{
	odf_color = boost::none;

	unsigned char ucA=0, ucR=0, ucG=0, ucB=0;
	bool result = false;	

	if(color)//easy, faster,realy  !!
	{
		ucR = color->Get_R(); 
		ucB = color->Get_B(); 
		ucG = color->Get_G(); 
		ucA = color->Get_A(); 
		result = true;
	}
	if(theme_color)
	{
		OOX::CTheme * docx_theme= docx_document->GetTheme();
		int theme_ind = theme_color->GetValue();
		switch(theme_ind)
		{
			case SimpleTypes::themecolorLight1:
				result = docx_theme->m_oThemeElements.m_oClrScheme.m_oLt1.tryGetRgb(ucR, ucG, ucB, ucA); break;
			case SimpleTypes::themecolorLight2:
				result = docx_theme->m_oThemeElements.m_oClrScheme.m_oLt2.tryGetRgb(ucR, ucG, ucB, ucA); break;
			case SimpleTypes::themecolorDark1:
				result = docx_theme->m_oThemeElements.m_oClrScheme.m_oDk1.tryGetRgb(ucR, ucG, ucB, ucA); break;
			case SimpleTypes::themecolorDark2:
				result = docx_theme->m_oThemeElements.m_oClrScheme.m_oDk2.tryGetRgb(ucR, ucG, ucB, ucA); break;
			case SimpleTypes::themecolorAccent1:
				result = docx_theme->m_oThemeElements.m_oClrScheme.m_oAccent1.tryGetRgb(ucR, ucG, ucB, ucA); break;
			case SimpleTypes::themecolorAccent2:
				result = docx_theme->m_oThemeElements.m_oClrScheme.m_oAccent2.tryGetRgb(ucR, ucG, ucB, ucA); break;
			case SimpleTypes::themecolorAccent3:
				result = docx_theme->m_oThemeElements.m_oClrScheme.m_oAccent3.tryGetRgb(ucR, ucG, ucB, ucA); break;
			case SimpleTypes::themecolorAccent4:
				result = docx_theme->m_oThemeElements.m_oClrScheme.m_oAccent4.tryGetRgb(ucR, ucG, ucB, ucA); break;
			case SimpleTypes::themecolorAccent5:
				result = docx_theme->m_oThemeElements.m_oClrScheme.m_oAccent5.tryGetRgb(ucR, ucG, ucB, ucA); break;
			case SimpleTypes::themecolorAccent6:
				result = docx_theme->m_oThemeElements.m_oClrScheme.m_oAccent6.tryGetRgb(ucR, ucG, ucB, ucA); break;
			case SimpleTypes::themecolorFollowedHyperlink:
				result = docx_theme->m_oThemeElements.m_oClrScheme.m_oFolHlink.tryGetRgb(ucR, ucG, ucB, ucA); break;
			case SimpleTypes::themecolorHyperlink:
				result = docx_theme->m_oThemeElements.m_oClrScheme.m_oHlink.tryGetRgb(ucR, ucG, ucB, ucA); break;
		}
	}
	if (result == true)
	{
		if (theme_tint)
		{
			OOX::Drawing::CHslColor col;
			col.SetRGBA(ucR, ucG, ucB);
			double dH,  dS,  dL;
			col.GetHSL(dH, dS,dL);
			dL = dL * theme_tint->GetValue()/255. + (1 - theme_tint->GetValue()/255.); 
			col.SetHSL(dH, dS,dL);
			col.GetRGBA(ucR, ucG, ucB,ucA);
		}
		SimpleTypes::CHexColor<> *oRgbColor = new SimpleTypes::CHexColor<>(ucR,ucG,ucB);
		if (oRgbColor)
		{		
			odf_color = odf::color(std::wstring(L"#") + string2std_string(oRgbColor->ToString().Right(6)));
			delete oRgbColor;
		}
	}
}
void DocxConverter::convert(ComplexTypes::Word::CColor *color, _CP_OPT(odf::color) & odf_color)
{
	if (!color)return;
	convert(color->m_oVal.GetPointer(), color->m_oThemeColor.GetPointer(),color->m_oThemeTint.GetPointer(),color->m_oThemeShade.GetPointer(), odf_color);
}
void DocxConverter::convert_styles()
{
	if (!odt_context) return;

	OOX::CStyles * docx_styles = docx_document->GetStyles();
	
	if (!docx_styles)return;

	//nullable<OOX::CLatentStyles > m_oLatentStyles;

	convert(docx_styles->m_oDocDefaults.GetPointer());

	for (long i=0; i< docx_styles->m_arrStyle.GetSize(); i++)
	{
		convert(&docx_styles->m_arrStyle[i]);
	}

}

void DocxConverter::convert(OOX::Logic::CHyperlink *oox_hyperlink)
{
	if (oox_hyperlink == NULL)return;
	if (oox_hyperlink->m_oId.IsInit() == false) return;

	std::wstring ref = find_link_by_id(oox_hyperlink->m_oId->GetValue(),2);

	odt_context->start_hyperlink(ref);
	
	for (long i=0; i< oox_hyperlink->m_arrItems.GetSize(); i++)
	{
		convert(oox_hyperlink->m_arrItems[i]);
	}
	odt_context->end_hyperlink();
	//nullable<CString                                      > m_sAnchor;
	//nullable<CString                                      > m_sDocLocation;
	//nullable<SimpleTypes::COnOff<SimpleTypes::onoffFalse> > m_oHistory;
	//nullable<CString                                      > m_sTgtFrame;
	//nullable<CString                                      > m_sTooltip;

}

void DocxConverter::convert(OOX::CDocDefaults *def_style)
{
	if (def_style == NULL)return;

	if (def_style->m_oParPr.IsInit())
	{
		odt_context->styles_context()->create_default_style(odf::style_family::Paragraph);					
		odf::style_paragraph_properties	* paragraph_properties	= odt_context->styles_context()->last_state().get_paragraph_properties();

		convert(def_style->m_oParPr.GetPointer(), paragraph_properties/*, text_properties*/); 
	}
	if (def_style->m_oRunPr.IsInit())
	{
		odt_context->styles_context()->create_default_style(odf::style_family::Text);					
		odf::style_text_properties	* text_properties = odt_context->styles_context()->last_state().get_text_properties();

		convert(def_style->m_oRunPr.GetPointer(), text_properties);
	}

}

void DocxConverter::convert(OOX::CStyle	*oox_style)
{
	if (oox_style == NULL)return;

	if (oox_style->m_oType.IsInit() == false)return;
	odf::style_family family = odf::style_family::None;

	if ( SimpleTypes::styletypeNumbering == oox_style->m_oType->GetValue())
	{
		return;
	}
	
	switch(oox_style->m_oType->GetValue())
	{
	case SimpleTypes::styletypeCharacter : family = odf::style_family::Text;		break;
	case SimpleTypes::styletypeParagraph : family = odf::style_family::Paragraph;	break;
	case SimpleTypes::styletypeTable     : family = odf::style_family::Table;		break;
	}

	if (family == odf::style_family::None) return;

	std::wstring oox_name = oox_style->m_sStyleId.IsInit() ? string2std_string(*oox_style->m_sStyleId) : L"";

	odt_context->styles_context()->create_style(oox_name,family, false, true, -1); 

	if (oox_style->m_oName.IsInit() && oox_style->m_oName->m_sVal.IsInit()) 
		odt_context->styles_context()->last_state().set_display_name(string2std_string(*oox_style->m_oName->m_sVal));

	if (oox_style->m_oRunPr.IsInit())
	{
		odf::style_text_properties	* text_properties = odt_context->styles_context()->last_state().get_text_properties();

		convert(oox_style->m_oRunPr.GetPointer(), text_properties);
	}
	if (oox_style->m_oBasedOn.IsInit() && oox_style->m_oBasedOn->m_sVal.IsInit())
		odt_context->styles_context()->last_state().set_parent_style_name(string2std_string(*oox_style->m_oBasedOn->m_sVal));

		//nullable<SimpleTypes::COnOff<>     > m_oCustomStyle;
		//nullable<SimpleTypes::COnOff<>     > m_oDefault;

		//nullable<ComplexTypes::Word::CString_                       > m_oAliases;
		//nullable<ComplexTypes::Word::COnOff2<SimpleTypes::onoffTrue>> m_oAutoRedefine;
		//nullable<ComplexTypes::Word::COnOff2<SimpleTypes::onoffTrue>> m_oHidden;
		//nullable<ComplexTypes::Word::COnOff2<SimpleTypes::onoffTrue>> m_oLocked;
		//nullable<ComplexTypes::Word::COnOff2<SimpleTypes::onoffTrue>> m_oPersonal;
		//nullable<ComplexTypes::Word::COnOff2<SimpleTypes::onoffTrue>> m_oPersonalCompose;
		//nullable<ComplexTypes::Word::COnOff2<SimpleTypes::onoffTrue>> m_oPersonalReply;
		//nullable<ComplexTypes::Word::COnOff2<SimpleTypes::onoffTrue>> m_oQFormat;
		//nullable<ComplexTypes::Word::COnOff2<SimpleTypes::onoffTrue>> m_oSemiHidden;
		//nullable<OOX::Logic::CTableProperty                         > m_oTblPr;		
		//CSimpleArray<OOX::Logic::CTableStyleProperties              > m_arrTblStylePr;
		//nullable<OOX::Logic::CTableCellProperties                   > m_oTcPr;
		//nullable<OOX::Logic::CTableRowProperties                    > m_oTrPr;
		//nullable<ComplexTypes::Word::CDecimalNumber                 > m_oUiPriority;
		//nullable<ComplexTypes::Word::COnOff2<SimpleTypes::onoffTrue>> m_oUnhideWhenUsed;

}

void DocxConverter::convert(OOX::Logic::CCommentRangeStart* oox_comm_start)
{
	if(oox_comm_start == NULL)return;
	if (oox_comm_start->m_oId.IsInit() == false) return;

	int oox_comm_id = oox_comm_start->m_oId->GetValue();

	bool added = odt_context->start_comment(oox_comm_id);

	if (added==false)
	{
		convert_comment(oox_comm_id);
	}
}

void DocxConverter::convert(OOX::Logic::CCommentRangeEnd* oox_comm_end)
{
	if(oox_comm_end == NULL)return;
	if (oox_comm_end->m_oId.IsInit() == false) return;

	int oox_comm_id = oox_comm_end->m_oId->GetValue();

	 odt_context->end_comment(oox_comm_id);
}
void DocxConverter::convert(OOX::Logic::CCommentReference* oox_comm_ref)
{
	if(oox_comm_ref == NULL)return;
	if (oox_comm_ref->m_oId.IsInit() == false) return;

	int oox_comm_id = oox_comm_ref->m_oId->GetValue();

	bool added = odt_context->start_comment(oox_comm_id);

	if (added == false)
	{
		//������ ����� ���� � �� �� RangeStart
		convert_comment(oox_comm_id);
	}

}
void DocxConverter::convert_comment(int oox_comm_id)
{
	OOX::CComments * docx_comments = docx_document->GetComments();
	if (!docx_comments)return;

	for (int comm =0 ; comm < docx_comments->m_arrComments.GetSize(); comm++)
	{
		OOX::CComment* oox_comment = docx_comments->m_arrComments[comm];
		if (oox_comment == NULL) continue;
		if (oox_comment->m_oId.IsInit() == false) continue;
		
		if (oox_comment->m_oId->GetValue() == oox_comm_id)
		{
			odt_context->start_comment_content();
			{
				if (oox_comment->m_oAuthor.IsInit())odt_context->comment_context()->set_author	(string2std_string(*oox_comment->m_oAuthor));
				if (oox_comment->m_oDate.IsInit())	odt_context->comment_context()->set_date	(string2std_string(oox_comment->m_oDate->GetValue()));
				if (oox_comment->m_oInitials.IsInit()){}

				for (long i=0; i <oox_comment->m_arrItems.GetSize(); i++)
				{
					convert(oox_comment->m_arrItems[i]);
				}
			}
			odt_context->end_comment_content();
		}
	}
}

} 

