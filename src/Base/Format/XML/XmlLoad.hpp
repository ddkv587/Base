#ifndef __XMLLOADH__
#define __XMLLOADH__

#include "Typedef.h"

enum EDataType 
{
    DT_INT = 0,
    DT_UINT,
    DT_STRING,
    DT_BOOLEAN
};

struct tagXMLDocument
{
    UINT    uiChildSize;
    struct tagXMLNode*  pChild;
};

struct tagXMLNode
{
    CHAR                    strName[128];
    CHAR                    strValue[64];  
    
    UINT                    uiChildSize;
    UINT                    uiAttributeSize;
    struct tagXMLAttribute* pAttribute;
    struct tagXMLNode*      pChild;
    struct tagXMLNode*      pNext;

    struct tagXMLNode*      pParent;
    struct tagXMLDocument*  pRoot;
};

struct tagXMLAttribute
{
    CHAR                    strName[128];
    CHAR                    strValue[64];  
    struct tagXMLNode*      pParent;
    struct tagXMLAttribute* pNext;  
};

extern BOOLEAN                  loadXML( const CHAR* strPath, struct tagXMLDocument* pDoc );
extern void                     showXML( struct tagXMLDocument* pDoc );
extern void                     releaseXML( struct tagXMLDocument* pDoc);
extern struct tagXMLNode*       getNode( const CHAR* strName, struct tagXMLDocument* pDoc );
extern struct tagXMLNode*       getChildNode( const CHAR* strName, struct tagXMLNode *pNode );
extern struct tagXMLAttribute*  getAttribute( const CHAR *strName, struct tagXMLNode *pNode );
#endif