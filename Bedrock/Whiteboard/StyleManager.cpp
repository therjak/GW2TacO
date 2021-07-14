#include "StyleManager.h"

#include "../BaseLib/string_format.h"

CStyleManager::CStyleManager( void )
{
}


CStyleManager::~CStyleManager( void )
{
}

void CStyleManager::ParseDeclarations(
    std::string_view s,
    std::unordered_map<std::string, std::string>& dRuleset) {
  auto propertiesArr = Split(s,_T( ";" ));
  for (const auto& p: propertiesArr) {
    auto prop = Split(p,_T( ":" ));
    if (prop.size() != 2) continue;
    std::string key(Trim(prop[0]));
    std::string value(Trim(prop[1]));
    dRuleset[key] = value;
  }
}

bool CStyleManager::ParseStyleData(std::string_view s) {
  int nPos = 0;
  std::string style(s);
  while ((nPos = style.find(_T( "/*" ))) != std::string::npos)
  {
    int nEnd = style.find( _T( "*/" ), nPos );
    if (nEnd == std::string::npos)
      nEnd = style.size();

    style = style.substr( 0, nPos ) + style.substr( nEnd + 2 );
  }

  auto rules = Split(style, _T( "}" ) );
  for ( const auto& r: rules)
  {
    auto properties = Split(r, _T( "{" ) );
    if ( properties.size() != 2 ) continue;

    std::unordered_map<std::string, std::string> dRuleset;
    ParseDeclarations( properties[ 1 ], dRuleset );

    auto selectors = Split(properties[ 0 ], _T( "," ) );
    for ( const auto s: selectors )
    {
      std::string selector(Trim(s));
      auto& mr = dRules[selector];
      for (auto& or : dRuleset) {
        mr.insert_or_assign(or.first, or.second);
      }
    }
  }
  return true;
}

void CStyleManager::Reset()
{
  dRules.clear();
}

void CStyleManager::CollectElementsBySimpleSelector(CWBItem* pItem,
                                                    CArray<CWBItem*>& itemset,
                                                    std::string_view selector,
                                                    bool bIncludeRoot) {
  if ( bIncludeRoot && pItem->IsFitForSelector( selector ) )
    itemset.Add( pItem );

  for ( unsigned int i = 0; i < pItem->NumChildren(); i++ )
  {
    CollectElementsBySimpleSelector( (CWBItem *)pItem->GetChild( i ), itemset, selector, true );
  }
}

CArray<CWBItem*> CStyleManager::GetElementsBySelector(
    CWBItem* pRootItem, std::string_view selector) {
  CArray<CWBItem*> result;
  result.Add( (CWBItem *)pRootItem );
  auto components = Split(selector, _T( " " ) );
  for ( size_t i = 0; i < components.size(); i++ )
  {
    if ( components[ i ].size() < 1 ) continue;
    CArray<CWBItem*> narrowResult;
    for ( int j = 0; j < result.NumItems(); j++ )
    {
      CollectElementsBySimpleSelector( result[ j ], narrowResult, components[ i ], i == 0 );
    }
    result = narrowResult;
  }

  return result;
}

void CStyleManager::ApplyStyles( CWBItem * pRootItem )
{
  for ( auto& r: dRules )
  {

    std::string selector = r.first;
    std::unordered_map<std::string, std::string>& rules = r.second;

    auto PseudoTags = Split(selector, _T( ":" ));
    CArray<CWBItem*> items = GetElementsBySelector( pRootItem, PseudoTags[ 0 ] );

    for ( int j = 0; j < items.NumItems(); j++ )
    {
      for ( auto& ry: rules )
      {
        std::string rule = ry.first;
        std::string value = ry.second;

        if ( !items[ j ]->ApplyStyle( rule, value, PseudoTags ) )
        {
          LOG_DBG( "[css] rule %s was not handled by item '%s' #%s .%s", rule.c_str(), items[ j ]->GetType().c_str(), items[ j ]->GetID().c_str(), items[ j ]->GetClassString().c_str() );
        }
      }
    }
  }
}

void CStyleManager::ApplyStylesFromDeclarations(
    CWBItem* pRootItem, std::string_view sDeclarations) {
  std::unordered_map<std::string, std::string> dRuleset;
  ParseDeclarations( sDeclarations, dRuleset );

  for ( auto& r: dRuleset )
  {
    std::string rule = r.first;
    std::string value = r.second;
    auto PseudoTags = Split(rule, _T( ":" ) );

    if ( !pRootItem->ApplyStyle( rule, value, PseudoTags ) )
    {
      LOG_DBG( "[css] rule '%s' was not handled by item '%s' #%s .%s", rule.c_str(), pRootItem->GetType().c_str(), pRootItem->GetID().c_str(), pRootItem->GetClassString().c_str() );
    }
  }
}
