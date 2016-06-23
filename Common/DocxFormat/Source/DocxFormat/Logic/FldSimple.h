#pragma once
#ifndef OOX_LOGIC_FIELD_SIMPLE_INCLUDE_H_
#define OOX_LOGIC_FIELD_SIMPLE_INCLUDE_H_

#include "../../Base/Nullable.h"

#include "../WritingElement.h"
#include "../../Common/SimpleTypes_Word.h"
#include "../../Common/SimpleTypes_Shared.h"

namespace OOX
{
	namespace Logic
	{
		//--------------------------------------------------------------------------------
		// CFldSimple 17.16.19 (Part 1)
		//--------------------------------------------------------------------------------	
		class CFldSimple : public WritingElementWithChilds<>
		{
		public:
			CFldSimple()
			{
			}
			CFldSimple(XmlUtils::CXmlNode &oNode)
			{
				fromXML( oNode );
			}
			CFldSimple(XmlUtils::CXmlLiteReader& oReader)
			{
				fromXML( oReader );
			}
			virtual ~CFldSimple()
			{
			}

		public:

			const CFldSimple &operator =(const XmlUtils::CXmlNode& oNode)
			{
				ClearItems();
				fromXML( (XmlUtils::CXmlNode&)oNode );
				return *this;
			}

			const CFldSimple &operator =(const XmlUtils::CXmlLiteReader& oReader)
			{
				ClearItems();
				fromXML( (XmlUtils::CXmlLiteReader&)oReader );
				return *this;
			}

			virtual void ClearItems()
			{
				m_oDirty.SetValue( SimpleTypes::onoffFalse );
				m_oFldLock.SetValue( SimpleTypes::onoffFalse );
				m_sInstr.reset();

				WritingElementWithChilds::ClearItems();
			}


		public:

			virtual void         fromXML(XmlUtils::CXmlNode& oNode);
			virtual void         fromXML(XmlUtils::CXmlLiteReader& oReader);
			virtual CString      toXML() const;
			virtual EElementType getType() const
			{
				return et_w_fldSimple;
			}

		private:

			void ReadAttributes(XmlUtils::CXmlLiteReader& oReader)
			{
				// Читаем атрибуты
				WritingElement_ReadAttributes_Start( oReader )
				WritingElement_ReadAttributes_Read_if     ( oReader, _T("w:dirty"),   m_oDirty )
				WritingElement_ReadAttributes_Read_else_if( oReader, _T("w:fldLock"), m_oFldLock )
				WritingElement_ReadAttributes_Read_else_if( oReader, _T("w:instr"),   m_sInstr )
				WritingElement_ReadAttributes_End( oReader )
			}

		public:

			// Attributes
			SimpleTypes::COnOff<SimpleTypes::onoffFalse> m_oDirty;
			SimpleTypes::COnOff<SimpleTypes::onoffFalse> m_oFldLock;
			nullable<CString >                           m_sInstr;

			// Childs
		};

	} // namespace Logic
} // namespace OOX

#endif // OOX_LOGIC_FIELD_SIMPLE_INCLUDE_H_