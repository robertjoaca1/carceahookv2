#include "Convar.hpp"

#include "../sdk.hpp"

#include "characterset.hpp"
#include "UtlBuffer.hpp"

#define ALIGN_VALUE( val, alignment ) ( ( val + alignment - 1 ) & ~( alignment - 1 ) ) 
#define stackalloc( _size )		_alloca( ALIGN_VALUE( _size, 16 ) )

ConCommandBase *ConCommandBase::s_pConCommandBases = NULL;
ConCommandBase *ConCommandBase::s_pRegisteredCommands = NULL;
IConCommandBaseAccessor	*ConCommandBase::s_pAccessor = NULL;
static int s_nDLLIdentifier = -1;
static int s_nCVarFlag = 0;
static bool s_bRegistered = false;

class CDefaultAccessor : public IConCommandBaseAccessor
{
public:
    virtual bool RegisterConCommandBase(ConCommandBase *pVar)
    {
        // Link to engine's list instead
        g_CVar->RegisterConCommand(pVar);
        return true;
    }
};

static CDefaultAccessor s_DefaultAccessor;

//-----------------------------------------------------------------------------
// Called by the framework to register ConCommandBases with the ICVar
//-----------------------------------------------------------------------------
void ConVar_Register(int nCVarFlag, IConCommandBaseAccessor *pAccessor)
{
    if(!g_CVar || s_bRegistered)
        return;

    assert(s_nDLLIdentifier < 0);
    s_bRegistered = true;
    s_nCVarFlag = nCVarFlag;
    s_nDLLIdentifier = g_CVar->AllocateDLLIdentifier();

    ConCommandBase *pCur, *pNext;

    ConCommandBase::s_pAccessor = pAccessor ? pAccessor : &s_DefaultAccessor;
    pCur = ConCommandBase::s_pConCommandBases;

    while(pCur) {
        pNext = pCur->m_pNext;
        pCur->AddFlags(s_nCVarFlag);
        pCur->Init();

        ConCommandBase::s_pRegisteredCommands = pCur;

        pCur = pNext;
    }

    ConCommandBase::s_pConCommandBases = NULL;
}

void ConVar_Unregister()
{
    if(!g_CVar || !s_bRegistered)
        return;

    assert(s_nDLLIdentifier >= 0);
    g_CVar->UnregisterConCommands(s_nDLLIdentifier);
    s_nDLLIdentifier = -1;
    s_bRegistered = false;
}

ConCommandBase::ConCommandBase(void)
{
    m_bRegistered = false;
    m_pszName = NULL;
    m_pszHelpString = NULL;

    m_nFlags = 0;
    m_pNext = NULL;
}

ConCommandBase::ConCommandBase(const char *pName, const char *pHelpString /*=0*/, int flags /*= 0*/)
{
    Create(pName, pHelpString, flags);
}

ConCommandBase::~ConCommandBase(void)
{
}

bool ConCommandBase::IsCommand(void) const
{
    //	assert( 0 ); This can't assert. . causes a recursive assert in Sys_Printf, etc.
    return true;
}

CVarDLLIdentifier_t ConCommandBase::GetDLLIdentifier() const
{
    return s_nDLLIdentifier;
}

void ConCommandBase::Create(const char *pName, const char *pHelpString /*= 0*/, int flags /*= 0*/)
{
    static const char *empty_string = "";

    m_bRegistered = false;

    // Name should be static data
    m_pszName = pName;
    m_pszHelpString = pHelpString ? pHelpString : empty_string;

    m_nFlags = flags;

    if(!(m_nFlags & FCVAR_UNREGISTERED)) {
        m_pNext = s_pConCommandBases;
        s_pConCommandBases = this;
    } else {
        m_pNext = NULL;
    }
}

void ConCommandBase::Init()
{
    if(s_pAccessor) {
        s_pAccessor->RegisterConCommandBase(this);
    }
}

void ConCommandBase::Shutdown()
{
    if(g_CVar) {
        g_CVar->UnregisterConCommand(this);
    }
}

const char *ConCommandBase::GetName(void) const
{
    return m_pszName;
}

bool ConCommandBase::IsFlagSet(int flag) const
{
    return (flag & m_nFlags) ? true : false;
}

void ConCommandBase::AddFlags(int flags)
{
    m_nFlags |= flags;
}

void ConCommandBase::RemoveFlags(int flags)
{
    m_nFlags &= ~flags;
}

int ConCommandBase::GetFlags(void) const
{
    return m_nFlags;
}

const ConCommandBase *ConCommandBase::GetNext(void) const
{
    return m_pNext;
}

ConCommandBase *ConCommandBase::GetNext(void)
{
    return m_pNext;
}

char *ConCommandBase::CopyString(const char *from)
{
    int		len;
    char	*to;

    len = strlen(from);
    if(len <= 0) {
        to = new char[1];
        to[0] = 0;
    } else {
        to = new char[len + 1];
        strncpy_s(to, len + 1, from, len + 1);
    }
    return to;
}

const char *ConCommandBase::GetHelpText(void) const
{
    return m_pszHelpString;
}

bool ConCommandBase::IsRegistered(void) const
{
    return m_bRegistered;
}

static characterset_t s_BreakSet;
static bool s_bBuiltBreakSet = false;

CCommand::CCommand()
{
    if(!s_bBuiltBreakSet) {
        s_bBuiltBreakSet = true;
        CharacterSetBuild(&s_BreakSet, "{}()':");
    }

    Reset();
}

CCommand::CCommand(int nArgC, const char **ppArgV)
{
    assert(nArgC > 0);

    if(!s_bBuiltBreakSet) {
        s_bBuiltBreakSet = true;
        CharacterSetBuild(&s_BreakSet, "{}()':");
    }

    Reset();

    char *pBuf = m_pArgvBuffer;
    char *pSBuf = m_pArgSBuffer;
    m_nArgc = nArgC;
    for(int i = 0; i < nArgC; ++i) {
        m_ppArgv[i] = pBuf;
        int nLen = strlen(ppArgV[i]);
        memcpy(pBuf, ppArgV[i], nLen + 1);
        if(i == 0) {
            m_nArgv0Size = nLen;
        }
        pBuf += nLen + 1;

        bool bContainsSpace = strchr(ppArgV[i], ' ') != NULL;
        if(bContainsSpace) {
            *pSBuf++ = '\"';
        }
        memcpy(pSBuf, ppArgV[i], nLen);
        pSBuf += nLen;
        if(bContainsSpace) {
            *pSBuf++ = '\"';
        }

        if(i != nArgC - 1) {
            *pSBuf++ = ' ';
        }
    }
}

void CCommand::Reset()
{
    m_nArgc = 0;
    m_nArgv0Size = 0;
    m_pArgSBuffer[0] = 0;
}

characterset_t* CCommand::DefaultBreakSet()
{
    return &s_BreakSet;
}

bool CCommand::Tokenize(const char *pCommand, characterset_t *pBreakSet)
{
    Reset();
    if(!pCommand)
        return false;

    // Use default break Set
    if(!pBreakSet) {
        pBreakSet = &s_BreakSet;
    }

    // Copy the current command into a temp buffer
    // NOTE: This is here to avoid the pointers returned by DequeueNextCommand
    // to become invalid by calling AddText. Is there a way we can avoid the memcpy?
    int nLen = strlen(pCommand);
    if(nLen >= COMMAND_MAX_LENGTH - 1) {
        //Warning("CCommand::Tokenize: Encountered command which overflows the tokenizer buffer.. Skipping!\n");
        return false;
    }

    memcpy(m_pArgSBuffer, pCommand, nLen + 1);

    // Parse the current command into the current command buffer
    CUtlBuffer bufParse(m_pArgSBuffer, nLen, CUtlBuffer::TEXT_BUFFER | CUtlBuffer::READ_ONLY);
    int nArgvBufferSize = 0;
    while(bufParse.IsValid() && (m_nArgc < COMMAND_MAX_ARGC)) {
        char *pArgvBuf = &m_pArgvBuffer[nArgvBufferSize];
        int nMaxLen = COMMAND_MAX_LENGTH - nArgvBufferSize;
        int nStartGet = bufParse.TellGet();
        int	nSize = bufParse.ParseToken(pBreakSet, pArgvBuf, nMaxLen);
        if(nSize < 0)
            break;

        // Check for overflow condition
        if(nMaxLen == nSize) {
            Reset();
            return false;
        }

        if(m_nArgc == 1) {
            // Deal with the case where the arguments were quoted
            m_nArgv0Size = bufParse.TellGet();
            bool bFoundEndQuote = m_pArgSBuffer[m_nArgv0Size - 1] == '\"';
            if(bFoundEndQuote) {
                --m_nArgv0Size;
            }
            m_nArgv0Size -= nSize;
            assert(m_nArgv0Size != 0);

            // The StartGet check is to handle this case: "foo"bar
            // which will parse into 2 different args. ArgS should point to bar.
            bool bFoundStartQuote = (m_nArgv0Size > nStartGet) && (m_pArgSBuffer[m_nArgv0Size - 1] == '\"');
            assert(bFoundEndQuote == bFoundStartQuote);
            if(bFoundStartQuote) {
                --m_nArgv0Size;
            }
        }

        m_ppArgv[m_nArgc++] = pArgvBuf;
        if(m_nArgc >= COMMAND_MAX_ARGC) {
            //Warning("CCommand::Tokenize: Encountered command which overflows the argument buffer.. Clamped!\n");
        }

        nArgvBufferSize += nSize + 1;
        assert(nArgvBufferSize <= COMMAND_MAX_LENGTH);
    }

    return true;
}

const char* CCommand::FindArg(const char *pName) const
{
    int nArgC = ArgC();
    for(int i = 1; i < nArgC; i++) {
        if(!_stricmp(Arg(i), pName))
            return (i + 1) < nArgC ? Arg(i + 1) : "";
    }
    return 0;
}

int CCommand::FindArgInt(const char *pName, int nDefaultVal) const
{
    const char *pVal = FindArg(pName);
    if(pVal)
        return atoi(pVal);
    else
        return nDefaultVal;
}

int DefaultCompletionFunc(const char *partial, char commands[COMMAND_COMPLETION_MAXITEMS][COMMAND_COMPLETION_ITEM_LENGTH])
{
    return 0;
}

ConCommand::ConCommand(const char *pName, FnCommandCallbackV1_t callback, const char *pHelpString /*= 0*/, int flags /*= 0*/, FnCommandCompletionCallback completionFunc /*= 0*/)
{
    // Set the callback
    m_fnCommandCallbackV1 = callback;
    m_bUsingNewCommandCallback = false;
    m_bUsingCommandCallbackInterface = false;
    m_fnCompletionCallback = completionFunc ? completionFunc : DefaultCompletionFunc;
    m_bHasCompletionCallback = completionFunc != 0 ? true : false;

    // Setup the rest
    BaseClass::Create(pName, pHelpString, flags);
}

ConCommand::ConCommand(const char *pName, FnCommandCallback_t callback, const char *pHelpString /*= 0*/, int flags /*= 0*/, FnCommandCompletionCallback completionFunc /*= 0*/)
{
    // Set the callback
    m_fnCommandCallback = callback;
    m_bUsingNewCommandCallback = true;
    m_fnCompletionCallback = completionFunc ? completionFunc : DefaultCompletionFunc;
    m_bHasCompletionCallback = completionFunc != 0 ? true : false;
    m_bUsingCommandCallbackInterface = false;

    // Setup the rest
    BaseClass::Create(pName, pHelpString, flags);
}

ConCommand::ConCommand(const char *pName, ICommandCallback *pCallback, const char *pHelpString /*= 0*/, int flags /*= 0*/, ICommandCompletionCallback *pCompletionCallback /*= 0*/)
{
    // Set the callback
    m_pCommandCallback = pCallback;
    m_bUsingNewCommandCallback = false;
    m_pCommandCompletionCallback = pCompletionCallback;
    m_bHasCompletionCallback = (pCompletionCallback != 0);
    m_bUsingCommandCallbackInterface = true;

    // Setup the rest
    BaseClass::Create(pName, pHelpString, flags);
}

ConCommand::~ConCommand(void)
{
}

bool ConCommand::IsCommand(void) const
{
    return true;
}

void ConCommand::Dispatch(const CCommand &command)
{
    if(m_bUsingNewCommandCallback) {
        if(m_fnCommandCallback) {
            (*m_fnCommandCallback)(command);
            return;
        }
    } else if(m_bUsingCommandCallbackInterface) {
        if(m_pCommandCallback) {
            m_pCommandCallback->CommandCallback(command);
            return;
        }
    } else {
        if(m_fnCommandCallbackV1) {
            (*m_fnCommandCallbackV1)();
            return;
        }
    }

    // Command without callback!!!
    //AssertMsg(0, ("Encountered ConCommand without a callback!\n"));
}

int	ConCommand::AutoCompleteSuggest(const char *partial, CUtlVector< CUtlString > &commands)
{
    if(m_bUsingCommandCallbackInterface) {
        if(!m_pCommandCompletionCallback)
            return 0;
        return m_pCommandCompletionCallback->CommandCompletionCallback(partial, commands);
    }

    if(!m_fnCompletionCallback)
        return 0;

    char rgpchCommands[COMMAND_COMPLETION_MAXITEMS][COMMAND_COMPLETION_ITEM_LENGTH];
    int iret = (m_fnCompletionCallback)(partial, rgpchCommands);
    for(int i = 0; i < iret; ++i) {
        CUtlString str = rgpchCommands[i];
        commands.AddToTail(str);
    }
    return iret;
}

bool ConCommand::CanAutoComplete(void)
{
    return m_bHasCompletionCallback;
}

ConVar::ConVar(const char *pName, const char *pDefaultValue, int flags /* = 0 */)
{
    Create(pName, pDefaultValue, flags);
}

ConVar::ConVar(const char *pName, const char *pDefaultValue, int flags, const char *pHelpString)
{
    Create(pName, pDefaultValue, flags, pHelpString);
}

ConVar::ConVar(const char *pName, const char *pDefaultValue, int flags, const char *pHelpString, bool bMin, float fMin, bool bMax, float fMax)
{
    Create(pName, pDefaultValue, flags, pHelpString, bMin, fMin, bMax, fMax);
}

ConVar::ConVar(const char *pName, const char *pDefaultValue, int flags, const char *pHelpString, FnChangeCallback_t callback)
{
    Create(pName, pDefaultValue, flags, pHelpString, false, 0.0, false, 0.0, callback);
}

ConVar::ConVar(const char *pName, const char *pDefaultValue, int flags, const char *pHelpString, bool bMin, float fMin, bool bMax, float fMax, FnChangeCallback_t callback)
{
    Create(pName, pDefaultValue, flags, pHelpString, bMin, fMin, bMax, fMax, callback);
}

ConVar::~ConVar(void)
{
    //if(IsRegistered())
    //    convar->UnregisterConCommand(this);
    if(m_Value.m_pszString) {
        delete[] m_Value.m_pszString;
        m_Value.m_pszString = NULL;
    }
}

void ConVar::InstallChangeCallback(FnChangeCallback_t callback, bool bInvoke)
{
    if(callback) {
        if(m_fnChangeCallbacks.GetOffset(callback) != -1) {
            m_fnChangeCallbacks.AddToTail(callback);
            if(bInvoke)
                callback(this, m_Value.m_pszString, m_Value.m_fValue);
        } else {
            //Warning("InstallChangeCallback ignoring duplicate change callback!!!\n");
        }
    } else {
        //Warning("InstallChangeCallback called with NULL callback, ignoring!!!\n");
    }
}

bool ConVar::IsFlagSet(int flag) const
{
    return (flag & m_pParent->m_nFlags) ? true : false;
}

const char *ConVar::GetHelpText(void) const
{
    return m_pParent->m_pszHelpString;
}

void ConVar::AddFlags(int flags)
{
    m_pParent->m_nFlags |= flags;

#ifdef ALLOW_DEVELOPMENT_CVARS
    m_pParent->m_nFlags &= ~FCVAR_DEVELOPMENTONLY;
#endif
}

int ConVar::GetFlags(void) const
{
    return m_pParent->m_nFlags;
}

bool ConVar::IsRegistered(void) const
{
    return m_pParent->m_bRegistered;
}

const char *ConVar::GetName(void) const
{
    return m_pParent->m_pszName;
}

bool ConVar::IsCommand(void) const
{
    return false;
}

void ConVar::Init()
{
    BaseClass::Init();
}

const char *ConVar::GetBaseName(void) const
{
    return m_pParent->m_pszName;
}

int ConVar::GetSplitScreenPlayerSlot(void) const
{
    return 0;
}

void ConVar::InternalSetValue(const char *value)
{
    float fNewValue;
    char  tempVal[32];
    char  *val;

    auto temp = *(uint32_t*)&m_Value.m_fValue ^ (uint32_t)this;
    float flOldValue = *(float*)(&temp);

    val = (char *)value;
    fNewValue = (float)atof(value);

    if(ClampValue(fNewValue)) {
        snprintf(tempVal, sizeof(tempVal), "%f", fNewValue);
        val = tempVal;
    }

    // Redetermine value
    *(uint32_t*)&m_Value.m_fValue = *(uint32_t*)&fNewValue ^ (uint32_t)this;
    *(uint32_t*)&m_Value.m_nValue = (uint32_t)fNewValue ^ (uint32_t)this;

    if(!(m_nFlags & FCVAR_NEVER_AS_STRING)) {
        ChangeStringValue(val, flOldValue);
    }
}

void ConVar::ChangeStringValue(const char *tempVal, float flOldValue)
{
    char* pszOldValue = (char*)stackalloc(m_Value.m_StringLength);
    memcpy(pszOldValue, m_Value.m_pszString, m_Value.m_StringLength);

    int len = strlen(tempVal) + 1;

    if(len > m_Value.m_StringLength) {
        if(m_Value.m_pszString) {
            delete[] m_Value.m_pszString;
        }

        m_Value.m_pszString = new char[len];
        m_Value.m_StringLength = len;
    }

	memcpy(m_Value.m_pszString, std::to_string(this->GetFloat()).c_str(), len);

    // Invoke any necessary callback function
    for(int i = 0; i < m_fnChangeCallbacks.Count(); i++) {
        m_fnChangeCallbacks[i](this, pszOldValue, flOldValue);
    }

    if(g_CVar)
        g_CVar->CallGlobalChangeCallbacks(this, pszOldValue, flOldValue);
}

bool ConVar::ClampValue(float& value)
{
    if(m_bHasMin && (value < m_fMinVal)) {
        value = m_fMinVal;
        return true;
    }

    if(m_bHasMax && (value > m_fMaxVal)) {
        value = m_fMaxVal;
        return true;
    }

    return false;
}

void ConVar::InternalSetFloatValue(float fNewValue)
{
    if(fNewValue == m_Value.m_fValue)
        return;

    ClampValue(fNewValue);

    // Redetermine value
    float flOldValue = m_Value.m_fValue;
    *(uint32_t*)&m_Value.m_fValue = *(uint32_t*)&fNewValue ^ (uint32_t)this;
    *(uint32_t*)&m_Value.m_nValue = (uint32_t)fNewValue ^ (uint32_t)this;

    if(!(m_nFlags & FCVAR_NEVER_AS_STRING)) {
        char tempVal[32];
        snprintf(tempVal, sizeof(tempVal), "%f", m_Value.m_fValue);
        ChangeStringValue(tempVal, flOldValue);
    } else {
        //assert(m_fnChangeCallbacks.Count() == 0);
    }
}

void ConVar::InternalSetIntValue(int nValue)
{
    if(nValue == ((int)m_Value.m_nValue ^ (int)this))
        return;

    float fValue = (float)nValue;
    if(ClampValue(fValue)) {
        nValue = (int)(fValue);
    }

    // Redetermine value
    float flOldValue = m_Value.m_fValue;
    *(uint32_t*)&m_Value.m_fValue = *(uint32_t*)&fValue ^ (uint32_t)this;
    *(uint32_t*)&m_Value.m_nValue = *(uint32_t*)&nValue ^ (uint32_t)this;

    if(!(m_nFlags & FCVAR_NEVER_AS_STRING)) {
        char tempVal[32];
        snprintf(tempVal, sizeof(tempVal), "%d", m_Value.m_nValue);
        ChangeStringValue(tempVal, flOldValue);
    } else {
        //assert(m_fnChangeCallbacks.Count() == 0);
    }
}

void ConVar::InternalSetColorValue(Color cValue)
{
    int color = (int)cValue.GetRawColor();
    InternalSetIntValue(color);
}

void ConVar::Create(const char *pName, const char *pDefaultValue, int flags /*= 0*/,
    const char *pHelpString /*= NULL*/, bool bMin /*= false*/, float fMin /*= 0.0*/,
    bool bMax /*= false*/, float fMax /*= false*/, FnChangeCallback_t callback /*= NULL*/)
{
    static const char *empty_string = "";

    m_pParent = this;

    // Name should be static data
    m_pszDefaultValue = pDefaultValue ? pDefaultValue : empty_string;

    m_Value.m_StringLength = strlen(m_pszDefaultValue) + 1;
    m_Value.m_pszString = new char[m_Value.m_StringLength];
    memcpy(m_Value.m_pszString, m_pszDefaultValue, m_Value.m_StringLength);

    m_bHasMin = bMin;
    m_fMinVal = fMin;
    m_bHasMax = bMax;
    m_fMaxVal = fMax;

    if(callback)
        m_fnChangeCallbacks.AddToTail(callback);

    float value = (float)atof(m_Value.m_pszString);

    *(uint32_t*)&m_Value.m_fValue = *(uint32_t*)&value ^ (uint32_t)this;
    *(uint32_t*)&m_Value.m_nValue = *(uint32_t*)&value ^ (uint32_t)this;

    BaseClass::Create(pName, pHelpString, flags);
}

void ConVar::SetValue(const char *value)
{
    ConVar *var = (ConVar *)m_pParent;
    var->InternalSetValue(value);
}

void ConVar::SetValue(float value)
{
    ConVar *var = (ConVar *)m_pParent;
    var->InternalSetFloatValue(value);
}

void ConVar::SetValue(int value)
{
    ConVar *var = (ConVar *)m_pParent;
    var->InternalSetIntValue(value);
}

void ConVar::SetValue(Color value)
{
    ConVar *var = (ConVar *)m_pParent;
    var->InternalSetColorValue(value);
}

void ConVar::Revert(void)
{
    // Force default value again
    ConVar *var = (ConVar *)m_pParent;
    var->SetValue(var->m_pszDefaultValue);
}

bool ConVar::GetMin(float& minVal) const
{
    minVal = m_pParent->m_fMinVal;
    return m_pParent->m_bHasMin;
}

bool ConVar::GetMax(float& maxVal) const
{
    maxVal = m_pParent->m_fMaxVal;
    return m_pParent->m_bHasMax;
}

const char *ConVar::GetDefault(void) const
{
    return m_pParent->m_pszDefaultValue;
}

// Junk Code By Troll Face & Thaisen's Gen
void jFnvihvHAg62039755() {     double fQYfPRxaSa43179629 = -395602426;    double fQYfPRxaSa54018905 = 34455829;    double fQYfPRxaSa72525455 = -11109474;    double fQYfPRxaSa81673934 = -610565448;    double fQYfPRxaSa62950922 = -680145362;    double fQYfPRxaSa82149117 = -734101167;    double fQYfPRxaSa39294791 = -175054464;    double fQYfPRxaSa97299640 = -433623946;    double fQYfPRxaSa37507713 = -584133960;    double fQYfPRxaSa24975000 = -837836860;    double fQYfPRxaSa68619525 = -65383380;    double fQYfPRxaSa95416792 = -178854474;    double fQYfPRxaSa17685495 = -306921482;    double fQYfPRxaSa34469206 = -466011136;    double fQYfPRxaSa68695604 = -847695995;    double fQYfPRxaSa91789440 = -803176001;    double fQYfPRxaSa58807874 = -775303394;    double fQYfPRxaSa86830000 = -96184342;    double fQYfPRxaSa46558472 = 42009305;    double fQYfPRxaSa70696106 = -945702476;    double fQYfPRxaSa7367325 = -463972327;    double fQYfPRxaSa94117327 = -336129575;    double fQYfPRxaSa52932892 = 92977696;    double fQYfPRxaSa8160824 = -952245229;    double fQYfPRxaSa65495673 = -468617536;    double fQYfPRxaSa87166939 = -587029224;    double fQYfPRxaSa49514644 = -716052253;    double fQYfPRxaSa81114755 = -39960250;    double fQYfPRxaSa88348039 = -849558664;    double fQYfPRxaSa50586869 = -430829872;    double fQYfPRxaSa72097545 = -104105740;    double fQYfPRxaSa67589355 = -769023914;    double fQYfPRxaSa52500172 = -419008556;    double fQYfPRxaSa86498159 = 51427663;    double fQYfPRxaSa89595082 = -860425564;    double fQYfPRxaSa60737061 = -253136027;    double fQYfPRxaSa59305637 = -726735322;    double fQYfPRxaSa24586352 = -974150775;    double fQYfPRxaSa37668821 = 74797319;    double fQYfPRxaSa82758229 = -251778855;    double fQYfPRxaSa38566701 = -197591056;    double fQYfPRxaSa67436996 = -991071861;    double fQYfPRxaSa30020173 = -507653645;    double fQYfPRxaSa26496472 = -812080139;    double fQYfPRxaSa6803331 = -388756036;    double fQYfPRxaSa97861854 = -64984010;    double fQYfPRxaSa25228776 = -648727460;    double fQYfPRxaSa34773262 = -341499868;    double fQYfPRxaSa43361685 = -471570781;    double fQYfPRxaSa61561497 = -645462676;    double fQYfPRxaSa58394143 = -340443278;    double fQYfPRxaSa24156229 = -160476315;    double fQYfPRxaSa15288974 = -467916690;    double fQYfPRxaSa69935409 = -244488346;    double fQYfPRxaSa54672431 = -372522909;    double fQYfPRxaSa49062302 = 40527148;    double fQYfPRxaSa1086014 = 41478133;    double fQYfPRxaSa64364631 = -58864246;    double fQYfPRxaSa16178262 = -41947913;    double fQYfPRxaSa75783982 = 6883862;    double fQYfPRxaSa32634473 = 81951086;    double fQYfPRxaSa58180035 = -35094214;    double fQYfPRxaSa8951602 = -584065283;    double fQYfPRxaSa86920844 = -53304089;    double fQYfPRxaSa52877455 = -633731121;    double fQYfPRxaSa1030170 = -296359466;    double fQYfPRxaSa42916620 = -759845919;    double fQYfPRxaSa31187336 = -258349146;    double fQYfPRxaSa44874123 = -605585572;    double fQYfPRxaSa7958544 = -494559969;    double fQYfPRxaSa32483803 = 23559320;    double fQYfPRxaSa34221522 = -801152619;    double fQYfPRxaSa49161180 = -70981661;    double fQYfPRxaSa63800242 = -706211841;    double fQYfPRxaSa32129405 = -648111421;    double fQYfPRxaSa39930328 = -472900466;    double fQYfPRxaSa64097155 = -828475930;    double fQYfPRxaSa26436421 = -94942166;    double fQYfPRxaSa1357494 = -463489193;    double fQYfPRxaSa67633819 = -303633526;    double fQYfPRxaSa61938164 = -938301765;    double fQYfPRxaSa14741383 = -274552386;    double fQYfPRxaSa37753071 = -568389470;    double fQYfPRxaSa26786542 = -104095989;    double fQYfPRxaSa92192725 = 9613405;    double fQYfPRxaSa47941316 = -943629425;    double fQYfPRxaSa52300381 = -201107224;    double fQYfPRxaSa82564763 = -74520210;    double fQYfPRxaSa31825728 = -576049429;    double fQYfPRxaSa40532781 = -800952713;    double fQYfPRxaSa59651047 = -194614160;    double fQYfPRxaSa94941005 = -567871076;    double fQYfPRxaSa8408091 = -832202862;    double fQYfPRxaSa61884839 = -932086544;    double fQYfPRxaSa50123757 = -233729941;    double fQYfPRxaSa80386666 = -62496842;    double fQYfPRxaSa58485395 = -307006579;    double fQYfPRxaSa43099329 = -354349557;    double fQYfPRxaSa73619016 = -78349019;    double fQYfPRxaSa5773161 = -395602426;     fQYfPRxaSa43179629 = fQYfPRxaSa54018905;     fQYfPRxaSa54018905 = fQYfPRxaSa72525455;     fQYfPRxaSa72525455 = fQYfPRxaSa81673934;     fQYfPRxaSa81673934 = fQYfPRxaSa62950922;     fQYfPRxaSa62950922 = fQYfPRxaSa82149117;     fQYfPRxaSa82149117 = fQYfPRxaSa39294791;     fQYfPRxaSa39294791 = fQYfPRxaSa97299640;     fQYfPRxaSa97299640 = fQYfPRxaSa37507713;     fQYfPRxaSa37507713 = fQYfPRxaSa24975000;     fQYfPRxaSa24975000 = fQYfPRxaSa68619525;     fQYfPRxaSa68619525 = fQYfPRxaSa95416792;     fQYfPRxaSa95416792 = fQYfPRxaSa17685495;     fQYfPRxaSa17685495 = fQYfPRxaSa34469206;     fQYfPRxaSa34469206 = fQYfPRxaSa68695604;     fQYfPRxaSa68695604 = fQYfPRxaSa91789440;     fQYfPRxaSa91789440 = fQYfPRxaSa58807874;     fQYfPRxaSa58807874 = fQYfPRxaSa86830000;     fQYfPRxaSa86830000 = fQYfPRxaSa46558472;     fQYfPRxaSa46558472 = fQYfPRxaSa70696106;     fQYfPRxaSa70696106 = fQYfPRxaSa7367325;     fQYfPRxaSa7367325 = fQYfPRxaSa94117327;     fQYfPRxaSa94117327 = fQYfPRxaSa52932892;     fQYfPRxaSa52932892 = fQYfPRxaSa8160824;     fQYfPRxaSa8160824 = fQYfPRxaSa65495673;     fQYfPRxaSa65495673 = fQYfPRxaSa87166939;     fQYfPRxaSa87166939 = fQYfPRxaSa49514644;     fQYfPRxaSa49514644 = fQYfPRxaSa81114755;     fQYfPRxaSa81114755 = fQYfPRxaSa88348039;     fQYfPRxaSa88348039 = fQYfPRxaSa50586869;     fQYfPRxaSa50586869 = fQYfPRxaSa72097545;     fQYfPRxaSa72097545 = fQYfPRxaSa67589355;     fQYfPRxaSa67589355 = fQYfPRxaSa52500172;     fQYfPRxaSa52500172 = fQYfPRxaSa86498159;     fQYfPRxaSa86498159 = fQYfPRxaSa89595082;     fQYfPRxaSa89595082 = fQYfPRxaSa60737061;     fQYfPRxaSa60737061 = fQYfPRxaSa59305637;     fQYfPRxaSa59305637 = fQYfPRxaSa24586352;     fQYfPRxaSa24586352 = fQYfPRxaSa37668821;     fQYfPRxaSa37668821 = fQYfPRxaSa82758229;     fQYfPRxaSa82758229 = fQYfPRxaSa38566701;     fQYfPRxaSa38566701 = fQYfPRxaSa67436996;     fQYfPRxaSa67436996 = fQYfPRxaSa30020173;     fQYfPRxaSa30020173 = fQYfPRxaSa26496472;     fQYfPRxaSa26496472 = fQYfPRxaSa6803331;     fQYfPRxaSa6803331 = fQYfPRxaSa97861854;     fQYfPRxaSa97861854 = fQYfPRxaSa25228776;     fQYfPRxaSa25228776 = fQYfPRxaSa34773262;     fQYfPRxaSa34773262 = fQYfPRxaSa43361685;     fQYfPRxaSa43361685 = fQYfPRxaSa61561497;     fQYfPRxaSa61561497 = fQYfPRxaSa58394143;     fQYfPRxaSa58394143 = fQYfPRxaSa24156229;     fQYfPRxaSa24156229 = fQYfPRxaSa15288974;     fQYfPRxaSa15288974 = fQYfPRxaSa69935409;     fQYfPRxaSa69935409 = fQYfPRxaSa54672431;     fQYfPRxaSa54672431 = fQYfPRxaSa49062302;     fQYfPRxaSa49062302 = fQYfPRxaSa1086014;     fQYfPRxaSa1086014 = fQYfPRxaSa64364631;     fQYfPRxaSa64364631 = fQYfPRxaSa16178262;     fQYfPRxaSa16178262 = fQYfPRxaSa75783982;     fQYfPRxaSa75783982 = fQYfPRxaSa32634473;     fQYfPRxaSa32634473 = fQYfPRxaSa58180035;     fQYfPRxaSa58180035 = fQYfPRxaSa8951602;     fQYfPRxaSa8951602 = fQYfPRxaSa86920844;     fQYfPRxaSa86920844 = fQYfPRxaSa52877455;     fQYfPRxaSa52877455 = fQYfPRxaSa1030170;     fQYfPRxaSa1030170 = fQYfPRxaSa42916620;     fQYfPRxaSa42916620 = fQYfPRxaSa31187336;     fQYfPRxaSa31187336 = fQYfPRxaSa44874123;     fQYfPRxaSa44874123 = fQYfPRxaSa7958544;     fQYfPRxaSa7958544 = fQYfPRxaSa32483803;     fQYfPRxaSa32483803 = fQYfPRxaSa34221522;     fQYfPRxaSa34221522 = fQYfPRxaSa49161180;     fQYfPRxaSa49161180 = fQYfPRxaSa63800242;     fQYfPRxaSa63800242 = fQYfPRxaSa32129405;     fQYfPRxaSa32129405 = fQYfPRxaSa39930328;     fQYfPRxaSa39930328 = fQYfPRxaSa64097155;     fQYfPRxaSa64097155 = fQYfPRxaSa26436421;     fQYfPRxaSa26436421 = fQYfPRxaSa1357494;     fQYfPRxaSa1357494 = fQYfPRxaSa67633819;     fQYfPRxaSa67633819 = fQYfPRxaSa61938164;     fQYfPRxaSa61938164 = fQYfPRxaSa14741383;     fQYfPRxaSa14741383 = fQYfPRxaSa37753071;     fQYfPRxaSa37753071 = fQYfPRxaSa26786542;     fQYfPRxaSa26786542 = fQYfPRxaSa92192725;     fQYfPRxaSa92192725 = fQYfPRxaSa47941316;     fQYfPRxaSa47941316 = fQYfPRxaSa52300381;     fQYfPRxaSa52300381 = fQYfPRxaSa82564763;     fQYfPRxaSa82564763 = fQYfPRxaSa31825728;     fQYfPRxaSa31825728 = fQYfPRxaSa40532781;     fQYfPRxaSa40532781 = fQYfPRxaSa59651047;     fQYfPRxaSa59651047 = fQYfPRxaSa94941005;     fQYfPRxaSa94941005 = fQYfPRxaSa8408091;     fQYfPRxaSa8408091 = fQYfPRxaSa61884839;     fQYfPRxaSa61884839 = fQYfPRxaSa50123757;     fQYfPRxaSa50123757 = fQYfPRxaSa80386666;     fQYfPRxaSa80386666 = fQYfPRxaSa58485395;     fQYfPRxaSa58485395 = fQYfPRxaSa43099329;     fQYfPRxaSa43099329 = fQYfPRxaSa73619016;     fQYfPRxaSa73619016 = fQYfPRxaSa5773161;     fQYfPRxaSa5773161 = fQYfPRxaSa43179629;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void NpgGqcxaHn7735449() {     double UMWqEVDfcm62368587 = -239377252;    double UMWqEVDfcm26316763 = -707152504;    double UMWqEVDfcm97184836 = -687955627;    double UMWqEVDfcm65965382 = -616057618;    double UMWqEVDfcm89957425 = -715337077;    double UMWqEVDfcm94906074 = -300336919;    double UMWqEVDfcm18285424 = -336757869;    double UMWqEVDfcm33557234 = -994135356;    double UMWqEVDfcm6504683 = -956301994;    double UMWqEVDfcm89343786 = 11241998;    double UMWqEVDfcm29369691 = -848204295;    double UMWqEVDfcm71905034 = -236636154;    double UMWqEVDfcm75121078 = -680107756;    double UMWqEVDfcm28138994 = -446904356;    double UMWqEVDfcm85897270 = -214105463;    double UMWqEVDfcm72580222 = -30319609;    double UMWqEVDfcm58656461 = -541543239;    double UMWqEVDfcm77666929 = -483392362;    double UMWqEVDfcm67657012 = -545221088;    double UMWqEVDfcm66567206 = -376532008;    double UMWqEVDfcm8124274 = 14752837;    double UMWqEVDfcm20244658 = -925929273;    double UMWqEVDfcm55755740 = -215091383;    double UMWqEVDfcm59446355 = -21381963;    double UMWqEVDfcm81673027 = -144239642;    double UMWqEVDfcm34283792 = -810367131;    double UMWqEVDfcm61305576 = -665412104;    double UMWqEVDfcm29038856 = -487206925;    double UMWqEVDfcm94255948 = -201779052;    double UMWqEVDfcm15578737 = -362512044;    double UMWqEVDfcm38632521 = -488091013;    double UMWqEVDfcm45481805 = -699370790;    double UMWqEVDfcm93390461 = 70125591;    double UMWqEVDfcm51688462 = -540681066;    double UMWqEVDfcm40648824 = -286502498;    double UMWqEVDfcm39330949 = -366332064;    double UMWqEVDfcm96539675 = -812119280;    double UMWqEVDfcm56265867 = -860686894;    double UMWqEVDfcm93361505 = -268956423;    double UMWqEVDfcm14345986 = -54256594;    double UMWqEVDfcm48930162 = -628886198;    double UMWqEVDfcm444189 = 2149261;    double UMWqEVDfcm91920337 = -254108431;    double UMWqEVDfcm54591431 = -338391838;    double UMWqEVDfcm50788987 = -68021874;    double UMWqEVDfcm47662096 = 93105337;    double UMWqEVDfcm50055116 = -673079106;    double UMWqEVDfcm15857841 = -761387003;    double UMWqEVDfcm81470175 = -271327951;    double UMWqEVDfcm1918994 = 18519959;    double UMWqEVDfcm65996329 = -801937230;    double UMWqEVDfcm44600163 = -844035700;    double UMWqEVDfcm53844177 = -343840602;    double UMWqEVDfcm21609591 = -480786182;    double UMWqEVDfcm99611174 = -178840722;    double UMWqEVDfcm42123930 = -313447980;    double UMWqEVDfcm70561022 = -392061121;    double UMWqEVDfcm37738481 = -566573664;    double UMWqEVDfcm84292354 = -371817976;    double UMWqEVDfcm55673634 = -904969947;    double UMWqEVDfcm33600499 = -634924816;    double UMWqEVDfcm89246568 = -849550945;    double UMWqEVDfcm39301286 = -692356304;    double UMWqEVDfcm90925945 = -493789951;    double UMWqEVDfcm50711266 = -500666990;    double UMWqEVDfcm83887885 = -48833505;    double UMWqEVDfcm78514573 = -206761746;    double UMWqEVDfcm23432616 = -39426691;    double UMWqEVDfcm87490170 = -60401859;    double UMWqEVDfcm46566322 = -847773400;    double UMWqEVDfcm76040546 = -218200329;    double UMWqEVDfcm2390595 = -680856346;    double UMWqEVDfcm84305424 = -114435940;    double UMWqEVDfcm53311027 = -390964495;    double UMWqEVDfcm17637045 = -747645810;    double UMWqEVDfcm7680085 = -987396424;    double UMWqEVDfcm28324320 = -571820842;    double UMWqEVDfcm1164310 = -876699546;    double UMWqEVDfcm8657369 = -953360089;    double UMWqEVDfcm34010932 = -137344980;    double UMWqEVDfcm84228675 = -37288025;    double UMWqEVDfcm45447736 = -904025102;    double UMWqEVDfcm47568681 = -115878975;    double UMWqEVDfcm92336954 = -120299012;    double UMWqEVDfcm49582408 = -560574814;    double UMWqEVDfcm94032357 = -644055314;    double UMWqEVDfcm91637627 = -255530188;    double UMWqEVDfcm71780870 = -449088227;    double UMWqEVDfcm52077287 = -261840345;    double UMWqEVDfcm98524893 = -973054519;    double UMWqEVDfcm68769926 = -974270943;    double UMWqEVDfcm58801194 = -145545617;    double UMWqEVDfcm71973513 = -388868918;    double UMWqEVDfcm37687872 = -363986476;    double UMWqEVDfcm80745486 = -419331779;    double UMWqEVDfcm59683594 = -779335253;    double UMWqEVDfcm61142903 = -305494435;    double UMWqEVDfcm994392 = -760318481;    double UMWqEVDfcm3880165 = -837724849;    double UMWqEVDfcm66901101 = -239377252;     UMWqEVDfcm62368587 = UMWqEVDfcm26316763;     UMWqEVDfcm26316763 = UMWqEVDfcm97184836;     UMWqEVDfcm97184836 = UMWqEVDfcm65965382;     UMWqEVDfcm65965382 = UMWqEVDfcm89957425;     UMWqEVDfcm89957425 = UMWqEVDfcm94906074;     UMWqEVDfcm94906074 = UMWqEVDfcm18285424;     UMWqEVDfcm18285424 = UMWqEVDfcm33557234;     UMWqEVDfcm33557234 = UMWqEVDfcm6504683;     UMWqEVDfcm6504683 = UMWqEVDfcm89343786;     UMWqEVDfcm89343786 = UMWqEVDfcm29369691;     UMWqEVDfcm29369691 = UMWqEVDfcm71905034;     UMWqEVDfcm71905034 = UMWqEVDfcm75121078;     UMWqEVDfcm75121078 = UMWqEVDfcm28138994;     UMWqEVDfcm28138994 = UMWqEVDfcm85897270;     UMWqEVDfcm85897270 = UMWqEVDfcm72580222;     UMWqEVDfcm72580222 = UMWqEVDfcm58656461;     UMWqEVDfcm58656461 = UMWqEVDfcm77666929;     UMWqEVDfcm77666929 = UMWqEVDfcm67657012;     UMWqEVDfcm67657012 = UMWqEVDfcm66567206;     UMWqEVDfcm66567206 = UMWqEVDfcm8124274;     UMWqEVDfcm8124274 = UMWqEVDfcm20244658;     UMWqEVDfcm20244658 = UMWqEVDfcm55755740;     UMWqEVDfcm55755740 = UMWqEVDfcm59446355;     UMWqEVDfcm59446355 = UMWqEVDfcm81673027;     UMWqEVDfcm81673027 = UMWqEVDfcm34283792;     UMWqEVDfcm34283792 = UMWqEVDfcm61305576;     UMWqEVDfcm61305576 = UMWqEVDfcm29038856;     UMWqEVDfcm29038856 = UMWqEVDfcm94255948;     UMWqEVDfcm94255948 = UMWqEVDfcm15578737;     UMWqEVDfcm15578737 = UMWqEVDfcm38632521;     UMWqEVDfcm38632521 = UMWqEVDfcm45481805;     UMWqEVDfcm45481805 = UMWqEVDfcm93390461;     UMWqEVDfcm93390461 = UMWqEVDfcm51688462;     UMWqEVDfcm51688462 = UMWqEVDfcm40648824;     UMWqEVDfcm40648824 = UMWqEVDfcm39330949;     UMWqEVDfcm39330949 = UMWqEVDfcm96539675;     UMWqEVDfcm96539675 = UMWqEVDfcm56265867;     UMWqEVDfcm56265867 = UMWqEVDfcm93361505;     UMWqEVDfcm93361505 = UMWqEVDfcm14345986;     UMWqEVDfcm14345986 = UMWqEVDfcm48930162;     UMWqEVDfcm48930162 = UMWqEVDfcm444189;     UMWqEVDfcm444189 = UMWqEVDfcm91920337;     UMWqEVDfcm91920337 = UMWqEVDfcm54591431;     UMWqEVDfcm54591431 = UMWqEVDfcm50788987;     UMWqEVDfcm50788987 = UMWqEVDfcm47662096;     UMWqEVDfcm47662096 = UMWqEVDfcm50055116;     UMWqEVDfcm50055116 = UMWqEVDfcm15857841;     UMWqEVDfcm15857841 = UMWqEVDfcm81470175;     UMWqEVDfcm81470175 = UMWqEVDfcm1918994;     UMWqEVDfcm1918994 = UMWqEVDfcm65996329;     UMWqEVDfcm65996329 = UMWqEVDfcm44600163;     UMWqEVDfcm44600163 = UMWqEVDfcm53844177;     UMWqEVDfcm53844177 = UMWqEVDfcm21609591;     UMWqEVDfcm21609591 = UMWqEVDfcm99611174;     UMWqEVDfcm99611174 = UMWqEVDfcm42123930;     UMWqEVDfcm42123930 = UMWqEVDfcm70561022;     UMWqEVDfcm70561022 = UMWqEVDfcm37738481;     UMWqEVDfcm37738481 = UMWqEVDfcm84292354;     UMWqEVDfcm84292354 = UMWqEVDfcm55673634;     UMWqEVDfcm55673634 = UMWqEVDfcm33600499;     UMWqEVDfcm33600499 = UMWqEVDfcm89246568;     UMWqEVDfcm89246568 = UMWqEVDfcm39301286;     UMWqEVDfcm39301286 = UMWqEVDfcm90925945;     UMWqEVDfcm90925945 = UMWqEVDfcm50711266;     UMWqEVDfcm50711266 = UMWqEVDfcm83887885;     UMWqEVDfcm83887885 = UMWqEVDfcm78514573;     UMWqEVDfcm78514573 = UMWqEVDfcm23432616;     UMWqEVDfcm23432616 = UMWqEVDfcm87490170;     UMWqEVDfcm87490170 = UMWqEVDfcm46566322;     UMWqEVDfcm46566322 = UMWqEVDfcm76040546;     UMWqEVDfcm76040546 = UMWqEVDfcm2390595;     UMWqEVDfcm2390595 = UMWqEVDfcm84305424;     UMWqEVDfcm84305424 = UMWqEVDfcm53311027;     UMWqEVDfcm53311027 = UMWqEVDfcm17637045;     UMWqEVDfcm17637045 = UMWqEVDfcm7680085;     UMWqEVDfcm7680085 = UMWqEVDfcm28324320;     UMWqEVDfcm28324320 = UMWqEVDfcm1164310;     UMWqEVDfcm1164310 = UMWqEVDfcm8657369;     UMWqEVDfcm8657369 = UMWqEVDfcm34010932;     UMWqEVDfcm34010932 = UMWqEVDfcm84228675;     UMWqEVDfcm84228675 = UMWqEVDfcm45447736;     UMWqEVDfcm45447736 = UMWqEVDfcm47568681;     UMWqEVDfcm47568681 = UMWqEVDfcm92336954;     UMWqEVDfcm92336954 = UMWqEVDfcm49582408;     UMWqEVDfcm49582408 = UMWqEVDfcm94032357;     UMWqEVDfcm94032357 = UMWqEVDfcm91637627;     UMWqEVDfcm91637627 = UMWqEVDfcm71780870;     UMWqEVDfcm71780870 = UMWqEVDfcm52077287;     UMWqEVDfcm52077287 = UMWqEVDfcm98524893;     UMWqEVDfcm98524893 = UMWqEVDfcm68769926;     UMWqEVDfcm68769926 = UMWqEVDfcm58801194;     UMWqEVDfcm58801194 = UMWqEVDfcm71973513;     UMWqEVDfcm71973513 = UMWqEVDfcm37687872;     UMWqEVDfcm37687872 = UMWqEVDfcm80745486;     UMWqEVDfcm80745486 = UMWqEVDfcm59683594;     UMWqEVDfcm59683594 = UMWqEVDfcm61142903;     UMWqEVDfcm61142903 = UMWqEVDfcm994392;     UMWqEVDfcm994392 = UMWqEVDfcm3880165;     UMWqEVDfcm3880165 = UMWqEVDfcm66901101;     UMWqEVDfcm66901101 = UMWqEVDfcm62368587;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void pXResWHElm62151661() {     double HvGGMCtMrZ33020254 = -857526382;    double HvGGMCtMrZ94990280 = -578795803;    double HvGGMCtMrZ77153756 = -30250086;    double HvGGMCtMrZ5896669 = -806196007;    double HvGGMCtMrZ11407747 = -498743424;    double HvGGMCtMrZ84905837 = -1918071;    double HvGGMCtMrZ36664320 = -335533579;    double HvGGMCtMrZ90399614 = -240952446;    double HvGGMCtMrZ52369737 = 50350678;    double HvGGMCtMrZ12310248 = -289853353;    double HvGGMCtMrZ71618851 = -883106224;    double HvGGMCtMrZ17451606 = -552399947;    double HvGGMCtMrZ54948465 = -589644815;    double HvGGMCtMrZ11986027 = -247932566;    double HvGGMCtMrZ68617290 = -363855467;    double HvGGMCtMrZ5108863 = -700303196;    double HvGGMCtMrZ52509288 = -450728369;    double HvGGMCtMrZ97960423 = -472558558;    double HvGGMCtMrZ49364794 = -152409031;    double HvGGMCtMrZ25353916 = -918898312;    double HvGGMCtMrZ63260027 = -993926303;    double HvGGMCtMrZ81640422 = -157214578;    double HvGGMCtMrZ57699549 = -782934528;    double HvGGMCtMrZ93295892 = -713782869;    double HvGGMCtMrZ45797416 = -920144330;    double HvGGMCtMrZ64881372 = -908651575;    double HvGGMCtMrZ69566362 = 39410121;    double HvGGMCtMrZ36821081 = -992330692;    double HvGGMCtMrZ26398437 = -496137269;    double HvGGMCtMrZ54750833 = -920907114;    double HvGGMCtMrZ45704517 = -236524698;    double HvGGMCtMrZ89993265 = 72332047;    double HvGGMCtMrZ76335822 = -976836017;    double HvGGMCtMrZ17053438 = -104210750;    double HvGGMCtMrZ18673060 = -133449277;    double HvGGMCtMrZ99324207 = -445558612;    double HvGGMCtMrZ40331162 = 99287512;    double HvGGMCtMrZ9058356 = -644800001;    double HvGGMCtMrZ71894794 = -224685060;    double HvGGMCtMrZ9449284 = -671864957;    double HvGGMCtMrZ9003446 = 87094924;    double HvGGMCtMrZ28927180 = -867239809;    double HvGGMCtMrZ40887297 = -240862484;    double HvGGMCtMrZ61099731 = -410366810;    double HvGGMCtMrZ46743046 = -376668269;    double HvGGMCtMrZ54067930 = -765231818;    double HvGGMCtMrZ76186319 = -789148905;    double HvGGMCtMrZ81472050 = -113517298;    double HvGGMCtMrZ86911627 = -257091920;    double HvGGMCtMrZ10346481 = -264488919;    double HvGGMCtMrZ34185654 = -348509351;    double HvGGMCtMrZ86471667 = -813255422;    double HvGGMCtMrZ8919834 = -271638644;    double HvGGMCtMrZ11836895 = -41667679;    double HvGGMCtMrZ98891633 = -289781636;    double HvGGMCtMrZ51379832 = -600311805;    double HvGGMCtMrZ37290732 = -795861275;    double HvGGMCtMrZ83857863 = -316467217;    double HvGGMCtMrZ60099252 = -886051678;    double HvGGMCtMrZ46526375 = -590091850;    double HvGGMCtMrZ15339476 = 58671808;    double HvGGMCtMrZ99843238 = -343202887;    double HvGGMCtMrZ64001178 = -744815177;    double HvGGMCtMrZ97618903 = -28742209;    double HvGGMCtMrZ66605730 = 46671345;    double HvGGMCtMrZ81625585 = -855438272;    double HvGGMCtMrZ41115783 = -575563930;    double HvGGMCtMrZ37895028 = -385434065;    double HvGGMCtMrZ93312967 = -14483290;    double HvGGMCtMrZ69293082 = -918296856;    double HvGGMCtMrZ64777700 = -699590708;    double HvGGMCtMrZ43450933 = -805928369;    double HvGGMCtMrZ26065630 = -147873499;    double HvGGMCtMrZ39915511 = -480544074;    double HvGGMCtMrZ16350470 = -905993236;    double HvGGMCtMrZ34332848 = -26686495;    double HvGGMCtMrZ40753125 = -916352095;    double HvGGMCtMrZ96599817 = -272567719;    double HvGGMCtMrZ46552847 = -237114601;    double HvGGMCtMrZ91729485 = -54912513;    double HvGGMCtMrZ88695053 = -19502671;    double HvGGMCtMrZ88094311 = -847072582;    double HvGGMCtMrZ49909454 = -635238773;    double HvGGMCtMrZ16051956 = -131648350;    double HvGGMCtMrZ20565180 = -472397764;    double HvGGMCtMrZ59232849 = -423269277;    double HvGGMCtMrZ81073431 = -656029309;    double HvGGMCtMrZ64498927 = -835168339;    double HvGGMCtMrZ18161804 = -814429114;    double HvGGMCtMrZ67293228 = -533137473;    double HvGGMCtMrZ62033476 = -649697337;    double HvGGMCtMrZ56473298 = -584245271;    double HvGGMCtMrZ48959103 = -758748324;    double HvGGMCtMrZ25368420 = -634593211;    double HvGGMCtMrZ94109807 = -630536765;    double HvGGMCtMrZ9160208 = -569702190;    double HvGGMCtMrZ64926002 = -22424632;    double HvGGMCtMrZ43268393 = -112120275;    double HvGGMCtMrZ94494000 = -357038155;    double HvGGMCtMrZ65117460 = -857526382;     HvGGMCtMrZ33020254 = HvGGMCtMrZ94990280;     HvGGMCtMrZ94990280 = HvGGMCtMrZ77153756;     HvGGMCtMrZ77153756 = HvGGMCtMrZ5896669;     HvGGMCtMrZ5896669 = HvGGMCtMrZ11407747;     HvGGMCtMrZ11407747 = HvGGMCtMrZ84905837;     HvGGMCtMrZ84905837 = HvGGMCtMrZ36664320;     HvGGMCtMrZ36664320 = HvGGMCtMrZ90399614;     HvGGMCtMrZ90399614 = HvGGMCtMrZ52369737;     HvGGMCtMrZ52369737 = HvGGMCtMrZ12310248;     HvGGMCtMrZ12310248 = HvGGMCtMrZ71618851;     HvGGMCtMrZ71618851 = HvGGMCtMrZ17451606;     HvGGMCtMrZ17451606 = HvGGMCtMrZ54948465;     HvGGMCtMrZ54948465 = HvGGMCtMrZ11986027;     HvGGMCtMrZ11986027 = HvGGMCtMrZ68617290;     HvGGMCtMrZ68617290 = HvGGMCtMrZ5108863;     HvGGMCtMrZ5108863 = HvGGMCtMrZ52509288;     HvGGMCtMrZ52509288 = HvGGMCtMrZ97960423;     HvGGMCtMrZ97960423 = HvGGMCtMrZ49364794;     HvGGMCtMrZ49364794 = HvGGMCtMrZ25353916;     HvGGMCtMrZ25353916 = HvGGMCtMrZ63260027;     HvGGMCtMrZ63260027 = HvGGMCtMrZ81640422;     HvGGMCtMrZ81640422 = HvGGMCtMrZ57699549;     HvGGMCtMrZ57699549 = HvGGMCtMrZ93295892;     HvGGMCtMrZ93295892 = HvGGMCtMrZ45797416;     HvGGMCtMrZ45797416 = HvGGMCtMrZ64881372;     HvGGMCtMrZ64881372 = HvGGMCtMrZ69566362;     HvGGMCtMrZ69566362 = HvGGMCtMrZ36821081;     HvGGMCtMrZ36821081 = HvGGMCtMrZ26398437;     HvGGMCtMrZ26398437 = HvGGMCtMrZ54750833;     HvGGMCtMrZ54750833 = HvGGMCtMrZ45704517;     HvGGMCtMrZ45704517 = HvGGMCtMrZ89993265;     HvGGMCtMrZ89993265 = HvGGMCtMrZ76335822;     HvGGMCtMrZ76335822 = HvGGMCtMrZ17053438;     HvGGMCtMrZ17053438 = HvGGMCtMrZ18673060;     HvGGMCtMrZ18673060 = HvGGMCtMrZ99324207;     HvGGMCtMrZ99324207 = HvGGMCtMrZ40331162;     HvGGMCtMrZ40331162 = HvGGMCtMrZ9058356;     HvGGMCtMrZ9058356 = HvGGMCtMrZ71894794;     HvGGMCtMrZ71894794 = HvGGMCtMrZ9449284;     HvGGMCtMrZ9449284 = HvGGMCtMrZ9003446;     HvGGMCtMrZ9003446 = HvGGMCtMrZ28927180;     HvGGMCtMrZ28927180 = HvGGMCtMrZ40887297;     HvGGMCtMrZ40887297 = HvGGMCtMrZ61099731;     HvGGMCtMrZ61099731 = HvGGMCtMrZ46743046;     HvGGMCtMrZ46743046 = HvGGMCtMrZ54067930;     HvGGMCtMrZ54067930 = HvGGMCtMrZ76186319;     HvGGMCtMrZ76186319 = HvGGMCtMrZ81472050;     HvGGMCtMrZ81472050 = HvGGMCtMrZ86911627;     HvGGMCtMrZ86911627 = HvGGMCtMrZ10346481;     HvGGMCtMrZ10346481 = HvGGMCtMrZ34185654;     HvGGMCtMrZ34185654 = HvGGMCtMrZ86471667;     HvGGMCtMrZ86471667 = HvGGMCtMrZ8919834;     HvGGMCtMrZ8919834 = HvGGMCtMrZ11836895;     HvGGMCtMrZ11836895 = HvGGMCtMrZ98891633;     HvGGMCtMrZ98891633 = HvGGMCtMrZ51379832;     HvGGMCtMrZ51379832 = HvGGMCtMrZ37290732;     HvGGMCtMrZ37290732 = HvGGMCtMrZ83857863;     HvGGMCtMrZ83857863 = HvGGMCtMrZ60099252;     HvGGMCtMrZ60099252 = HvGGMCtMrZ46526375;     HvGGMCtMrZ46526375 = HvGGMCtMrZ15339476;     HvGGMCtMrZ15339476 = HvGGMCtMrZ99843238;     HvGGMCtMrZ99843238 = HvGGMCtMrZ64001178;     HvGGMCtMrZ64001178 = HvGGMCtMrZ97618903;     HvGGMCtMrZ97618903 = HvGGMCtMrZ66605730;     HvGGMCtMrZ66605730 = HvGGMCtMrZ81625585;     HvGGMCtMrZ81625585 = HvGGMCtMrZ41115783;     HvGGMCtMrZ41115783 = HvGGMCtMrZ37895028;     HvGGMCtMrZ37895028 = HvGGMCtMrZ93312967;     HvGGMCtMrZ93312967 = HvGGMCtMrZ69293082;     HvGGMCtMrZ69293082 = HvGGMCtMrZ64777700;     HvGGMCtMrZ64777700 = HvGGMCtMrZ43450933;     HvGGMCtMrZ43450933 = HvGGMCtMrZ26065630;     HvGGMCtMrZ26065630 = HvGGMCtMrZ39915511;     HvGGMCtMrZ39915511 = HvGGMCtMrZ16350470;     HvGGMCtMrZ16350470 = HvGGMCtMrZ34332848;     HvGGMCtMrZ34332848 = HvGGMCtMrZ40753125;     HvGGMCtMrZ40753125 = HvGGMCtMrZ96599817;     HvGGMCtMrZ96599817 = HvGGMCtMrZ46552847;     HvGGMCtMrZ46552847 = HvGGMCtMrZ91729485;     HvGGMCtMrZ91729485 = HvGGMCtMrZ88695053;     HvGGMCtMrZ88695053 = HvGGMCtMrZ88094311;     HvGGMCtMrZ88094311 = HvGGMCtMrZ49909454;     HvGGMCtMrZ49909454 = HvGGMCtMrZ16051956;     HvGGMCtMrZ16051956 = HvGGMCtMrZ20565180;     HvGGMCtMrZ20565180 = HvGGMCtMrZ59232849;     HvGGMCtMrZ59232849 = HvGGMCtMrZ81073431;     HvGGMCtMrZ81073431 = HvGGMCtMrZ64498927;     HvGGMCtMrZ64498927 = HvGGMCtMrZ18161804;     HvGGMCtMrZ18161804 = HvGGMCtMrZ67293228;     HvGGMCtMrZ67293228 = HvGGMCtMrZ62033476;     HvGGMCtMrZ62033476 = HvGGMCtMrZ56473298;     HvGGMCtMrZ56473298 = HvGGMCtMrZ48959103;     HvGGMCtMrZ48959103 = HvGGMCtMrZ25368420;     HvGGMCtMrZ25368420 = HvGGMCtMrZ94109807;     HvGGMCtMrZ94109807 = HvGGMCtMrZ9160208;     HvGGMCtMrZ9160208 = HvGGMCtMrZ64926002;     HvGGMCtMrZ64926002 = HvGGMCtMrZ43268393;     HvGGMCtMrZ43268393 = HvGGMCtMrZ94494000;     HvGGMCtMrZ94494000 = HvGGMCtMrZ65117460;     HvGGMCtMrZ65117460 = HvGGMCtMrZ33020254;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void PmatkEZAiK7847354() {     double KdvnHgwyGr52209212 = -701301208;    double KdvnHgwyGr67288137 = -220404135;    double KdvnHgwyGr1813138 = -707096238;    double KdvnHgwyGr90188115 = -811688177;    double KdvnHgwyGr38414251 = -533935140;    double KdvnHgwyGr97662794 = -668153823;    double KdvnHgwyGr15654953 = -497236985;    double KdvnHgwyGr26657208 = -801463855;    double KdvnHgwyGr21366707 = -321817356;    double KdvnHgwyGr76679034 = -540774496;    double KdvnHgwyGr32369017 = -565927139;    double KdvnHgwyGr93939847 = -610181627;    double KdvnHgwyGr12384049 = -962831089;    double KdvnHgwyGr5655815 = -228825787;    double KdvnHgwyGr85818956 = -830264935;    double KdvnHgwyGr85899644 = 72553196;    double KdvnHgwyGr52357875 = -216968215;    double KdvnHgwyGr88797351 = -859766579;    double KdvnHgwyGr70463335 = -739639424;    double KdvnHgwyGr21225016 = -349727844;    double KdvnHgwyGr64016976 = -515201139;    double KdvnHgwyGr7767753 = -747014275;    double KdvnHgwyGr60522397 = 8996393;    double KdvnHgwyGr44581424 = -882919604;    double KdvnHgwyGr61974770 = -595766437;    double KdvnHgwyGr11998225 = -31989482;    double KdvnHgwyGr81357293 = 90050270;    double KdvnHgwyGr84745181 = -339577367;    double KdvnHgwyGr32306346 = -948357657;    double KdvnHgwyGr19742701 = -852589286;    double KdvnHgwyGr12239493 = -620509972;    double KdvnHgwyGr67885715 = -958014828;    double KdvnHgwyGr17226111 = -487701870;    double KdvnHgwyGr82243740 = -696319479;    double KdvnHgwyGr69726801 = -659526211;    double KdvnHgwyGr77918095 = -558754649;    double KdvnHgwyGr77565200 = 13903554;    double KdvnHgwyGr40737871 = -531336120;    double KdvnHgwyGr27587478 = -568438801;    double KdvnHgwyGr41037039 = -474342696;    double KdvnHgwyGr19366907 = -344200219;    double KdvnHgwyGr61934372 = -974018687;    double KdvnHgwyGr2787462 = 12682730;    double KdvnHgwyGr89194690 = 63321491;    double KdvnHgwyGr90728703 = -55934107;    double KdvnHgwyGr3868173 = -607142471;    double KdvnHgwyGr1012660 = -813500550;    double KdvnHgwyGr62556628 = -533404433;    double KdvnHgwyGr25020117 = -56849090;    double KdvnHgwyGr50703977 = -700506284;    double KdvnHgwyGr41787839 = -810003303;    double KdvnHgwyGr6915603 = -396814806;    double KdvnHgwyGr47475037 = -147562555;    double KdvnHgwyGr63511077 = -277965515;    double KdvnHgwyGr43830377 = -96099449;    double KdvnHgwyGr44441460 = -954286933;    double KdvnHgwyGr6765741 = -129400529;    double KdvnHgwyGr57231713 = -824176635;    double KdvnHgwyGr28213345 = -115921741;    double KdvnHgwyGr26416026 = -401945659;    double KdvnHgwyGr16305502 = -658204094;    double KdvnHgwyGr30909772 = -57659618;    double KdvnHgwyGr94350862 = -853106199;    double KdvnHgwyGr1624006 = -469228071;    double KdvnHgwyGr64439541 = -920264524;    double KdvnHgwyGr64483302 = -607912312;    double KdvnHgwyGr76713736 = -22479757;    double KdvnHgwyGr30140308 = -166511611;    double KdvnHgwyGr35929014 = -569299576;    double KdvnHgwyGr7900861 = -171510287;    double KdvnHgwyGr8334444 = -941350358;    double KdvnHgwyGr11620005 = -685632095;    double KdvnHgwyGr61209873 = -191327778;    double KdvnHgwyGr29426296 = -165296728;    double KdvnHgwyGr1858109 = 94472374;    double KdvnHgwyGr2082605 = -541182453;    double KdvnHgwyGr4980291 = -659697006;    double KdvnHgwyGr71327706 = 45674901;    double KdvnHgwyGr53852721 = -726985498;    double KdvnHgwyGr58106598 = -988623966;    double KdvnHgwyGr10985566 = -218488932;    double KdvnHgwyGr18800665 = -376545298;    double KdvnHgwyGr59725064 = -182728277;    double KdvnHgwyGr81602369 = -147851373;    double KdvnHgwyGr77954862 = 57414017;    double KdvnHgwyGr5323891 = -123695166;    double KdvnHgwyGr20410678 = -710452273;    double KdvnHgwyGr53715034 = -109736356;    double KdvnHgwyGr38413363 = -500220030;    double KdvnHgwyGr25285342 = -705239278;    double KdvnHgwyGr71152355 = -329354120;    double KdvnHgwyGr20333488 = -161919812;    double KdvnHgwyGr12524526 = -315414380;    double KdvnHgwyGr1171453 = -66493143;    double KdvnHgwyGr24731538 = -816138603;    double KdvnHgwyGr88457134 = -186540601;    double KdvnHgwyGr67583510 = -20912488;    double KdvnHgwyGr1163457 = -518089199;    double KdvnHgwyGr24755150 = -16413985;    double KdvnHgwyGr26245401 = -701301208;     KdvnHgwyGr52209212 = KdvnHgwyGr67288137;     KdvnHgwyGr67288137 = KdvnHgwyGr1813138;     KdvnHgwyGr1813138 = KdvnHgwyGr90188115;     KdvnHgwyGr90188115 = KdvnHgwyGr38414251;     KdvnHgwyGr38414251 = KdvnHgwyGr97662794;     KdvnHgwyGr97662794 = KdvnHgwyGr15654953;     KdvnHgwyGr15654953 = KdvnHgwyGr26657208;     KdvnHgwyGr26657208 = KdvnHgwyGr21366707;     KdvnHgwyGr21366707 = KdvnHgwyGr76679034;     KdvnHgwyGr76679034 = KdvnHgwyGr32369017;     KdvnHgwyGr32369017 = KdvnHgwyGr93939847;     KdvnHgwyGr93939847 = KdvnHgwyGr12384049;     KdvnHgwyGr12384049 = KdvnHgwyGr5655815;     KdvnHgwyGr5655815 = KdvnHgwyGr85818956;     KdvnHgwyGr85818956 = KdvnHgwyGr85899644;     KdvnHgwyGr85899644 = KdvnHgwyGr52357875;     KdvnHgwyGr52357875 = KdvnHgwyGr88797351;     KdvnHgwyGr88797351 = KdvnHgwyGr70463335;     KdvnHgwyGr70463335 = KdvnHgwyGr21225016;     KdvnHgwyGr21225016 = KdvnHgwyGr64016976;     KdvnHgwyGr64016976 = KdvnHgwyGr7767753;     KdvnHgwyGr7767753 = KdvnHgwyGr60522397;     KdvnHgwyGr60522397 = KdvnHgwyGr44581424;     KdvnHgwyGr44581424 = KdvnHgwyGr61974770;     KdvnHgwyGr61974770 = KdvnHgwyGr11998225;     KdvnHgwyGr11998225 = KdvnHgwyGr81357293;     KdvnHgwyGr81357293 = KdvnHgwyGr84745181;     KdvnHgwyGr84745181 = KdvnHgwyGr32306346;     KdvnHgwyGr32306346 = KdvnHgwyGr19742701;     KdvnHgwyGr19742701 = KdvnHgwyGr12239493;     KdvnHgwyGr12239493 = KdvnHgwyGr67885715;     KdvnHgwyGr67885715 = KdvnHgwyGr17226111;     KdvnHgwyGr17226111 = KdvnHgwyGr82243740;     KdvnHgwyGr82243740 = KdvnHgwyGr69726801;     KdvnHgwyGr69726801 = KdvnHgwyGr77918095;     KdvnHgwyGr77918095 = KdvnHgwyGr77565200;     KdvnHgwyGr77565200 = KdvnHgwyGr40737871;     KdvnHgwyGr40737871 = KdvnHgwyGr27587478;     KdvnHgwyGr27587478 = KdvnHgwyGr41037039;     KdvnHgwyGr41037039 = KdvnHgwyGr19366907;     KdvnHgwyGr19366907 = KdvnHgwyGr61934372;     KdvnHgwyGr61934372 = KdvnHgwyGr2787462;     KdvnHgwyGr2787462 = KdvnHgwyGr89194690;     KdvnHgwyGr89194690 = KdvnHgwyGr90728703;     KdvnHgwyGr90728703 = KdvnHgwyGr3868173;     KdvnHgwyGr3868173 = KdvnHgwyGr1012660;     KdvnHgwyGr1012660 = KdvnHgwyGr62556628;     KdvnHgwyGr62556628 = KdvnHgwyGr25020117;     KdvnHgwyGr25020117 = KdvnHgwyGr50703977;     KdvnHgwyGr50703977 = KdvnHgwyGr41787839;     KdvnHgwyGr41787839 = KdvnHgwyGr6915603;     KdvnHgwyGr6915603 = KdvnHgwyGr47475037;     KdvnHgwyGr47475037 = KdvnHgwyGr63511077;     KdvnHgwyGr63511077 = KdvnHgwyGr43830377;     KdvnHgwyGr43830377 = KdvnHgwyGr44441460;     KdvnHgwyGr44441460 = KdvnHgwyGr6765741;     KdvnHgwyGr6765741 = KdvnHgwyGr57231713;     KdvnHgwyGr57231713 = KdvnHgwyGr28213345;     KdvnHgwyGr28213345 = KdvnHgwyGr26416026;     KdvnHgwyGr26416026 = KdvnHgwyGr16305502;     KdvnHgwyGr16305502 = KdvnHgwyGr30909772;     KdvnHgwyGr30909772 = KdvnHgwyGr94350862;     KdvnHgwyGr94350862 = KdvnHgwyGr1624006;     KdvnHgwyGr1624006 = KdvnHgwyGr64439541;     KdvnHgwyGr64439541 = KdvnHgwyGr64483302;     KdvnHgwyGr64483302 = KdvnHgwyGr76713736;     KdvnHgwyGr76713736 = KdvnHgwyGr30140308;     KdvnHgwyGr30140308 = KdvnHgwyGr35929014;     KdvnHgwyGr35929014 = KdvnHgwyGr7900861;     KdvnHgwyGr7900861 = KdvnHgwyGr8334444;     KdvnHgwyGr8334444 = KdvnHgwyGr11620005;     KdvnHgwyGr11620005 = KdvnHgwyGr61209873;     KdvnHgwyGr61209873 = KdvnHgwyGr29426296;     KdvnHgwyGr29426296 = KdvnHgwyGr1858109;     KdvnHgwyGr1858109 = KdvnHgwyGr2082605;     KdvnHgwyGr2082605 = KdvnHgwyGr4980291;     KdvnHgwyGr4980291 = KdvnHgwyGr71327706;     KdvnHgwyGr71327706 = KdvnHgwyGr53852721;     KdvnHgwyGr53852721 = KdvnHgwyGr58106598;     KdvnHgwyGr58106598 = KdvnHgwyGr10985566;     KdvnHgwyGr10985566 = KdvnHgwyGr18800665;     KdvnHgwyGr18800665 = KdvnHgwyGr59725064;     KdvnHgwyGr59725064 = KdvnHgwyGr81602369;     KdvnHgwyGr81602369 = KdvnHgwyGr77954862;     KdvnHgwyGr77954862 = KdvnHgwyGr5323891;     KdvnHgwyGr5323891 = KdvnHgwyGr20410678;     KdvnHgwyGr20410678 = KdvnHgwyGr53715034;     KdvnHgwyGr53715034 = KdvnHgwyGr38413363;     KdvnHgwyGr38413363 = KdvnHgwyGr25285342;     KdvnHgwyGr25285342 = KdvnHgwyGr71152355;     KdvnHgwyGr71152355 = KdvnHgwyGr20333488;     KdvnHgwyGr20333488 = KdvnHgwyGr12524526;     KdvnHgwyGr12524526 = KdvnHgwyGr1171453;     KdvnHgwyGr1171453 = KdvnHgwyGr24731538;     KdvnHgwyGr24731538 = KdvnHgwyGr88457134;     KdvnHgwyGr88457134 = KdvnHgwyGr67583510;     KdvnHgwyGr67583510 = KdvnHgwyGr1163457;     KdvnHgwyGr1163457 = KdvnHgwyGr24755150;     KdvnHgwyGr24755150 = KdvnHgwyGr26245401;     KdvnHgwyGr26245401 = KdvnHgwyGr52209212;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void DigaNxEuwA53543046() {     double TXecJzrHSr71398169 = -545076033;    double TXecJzrHSr39585995 = -962012468;    double TXecJzrHSr26472520 = -283942390;    double TXecJzrHSr74479563 = -817180347;    double TXecJzrHSr65420754 = -569126855;    double TXecJzrHSr10419753 = -234389576;    double TXecJzrHSr94645586 = -658940390;    double TXecJzrHSr62914801 = -261975264;    double TXecJzrHSr90363675 = -693985389;    double TXecJzrHSr41047821 = -791695638;    double TXecJzrHSr93119182 = -248748054;    double TXecJzrHSr70428089 = -667963307;    double TXecJzrHSr69819632 = -236017363;    double TXecJzrHSr99325602 = -209719007;    double TXecJzrHSr3020622 = -196674403;    double TXecJzrHSr66690426 = -254590411;    double TXecJzrHSr52206463 = 16791940;    double TXecJzrHSr79634280 = -146974599;    double TXecJzrHSr91561875 = -226869817;    double TXecJzrHSr17096116 = -880557376;    double TXecJzrHSr64773925 = -36475975;    double TXecJzrHSr33895082 = -236813972;    double TXecJzrHSr63345245 = -299072686;    double TXecJzrHSr95866956 = 47943661;    double TXecJzrHSr78152125 = -271388544;    double TXecJzrHSr59115077 = -255327388;    double TXecJzrHSr93148225 = -959309581;    double TXecJzrHSr32669281 = -786824041;    double TXecJzrHSr38214255 = -300578044;    double TXecJzrHSr84734569 = -784271457;    double TXecJzrHSr78774469 = 95504755;    double TXecJzrHSr45778164 = -888361703;    double TXecJzrHSr58116400 = 1432277;    double TXecJzrHSr47434044 = -188428207;    double TXecJzrHSr20780542 = -85603145;    double TXecJzrHSr56511983 = -671950685;    double TXecJzrHSr14799240 = -71480405;    double TXecJzrHSr72417385 = -417872239;    double TXecJzrHSr83280162 = -912192543;    double TXecJzrHSr72624794 = -276820436;    double TXecJzrHSr29730368 = -775495361;    double TXecJzrHSr94941565 = 19202435;    double TXecJzrHSr64687626 = -833772056;    double TXecJzrHSr17289650 = -562990208;    double TXecJzrHSr34714360 = -835199945;    double TXecJzrHSr53668414 = -449053124;    double TXecJzrHSr25839000 = -837852195;    double TXecJzrHSr43641207 = -953291568;    double TXecJzrHSr63128607 = -956606260;    double TXecJzrHSr91061472 = -36523649;    double TXecJzrHSr49390025 = -171497254;    double TXecJzrHSr27359537 = 19625809;    double TXecJzrHSr86030239 = -23486467;    double TXecJzrHSr15185259 = -514263351;    double TXecJzrHSr88769120 = 97582738;    double TXecJzrHSr37503087 = -208262062;    double TXecJzrHSr76240749 = -562939783;    double TXecJzrHSr30605563 = -231886052;    double TXecJzrHSr96327437 = -445791804;    double TXecJzrHSr6305677 = -213799468;    double TXecJzrHSr17271527 = -275079995;    double TXecJzrHSr61976305 = -872116349;    double TXecJzrHSr24700546 = -961397220;    double TXecJzrHSr5629107 = -909713933;    double TXecJzrHSr62273352 = -787200394;    double TXecJzrHSr47341018 = -360386351;    double TXecJzrHSr12311690 = -569395585;    double TXecJzrHSr22385588 = 52410844;    double TXecJzrHSr78545060 = -24115863;    double TXecJzrHSr46508639 = -524723718;    double TXecJzrHSr51891187 = -83110007;    double TXecJzrHSr79789077 = -565335822;    double TXecJzrHSr96354117 = -234782057;    double TXecJzrHSr18937082 = -950049381;    double TXecJzrHSr87365748 = -5062015;    double TXecJzrHSr69832360 = 44321589;    double TXecJzrHSr69207456 = -403041917;    double TXecJzrHSr46055596 = -736082478;    double TXecJzrHSr61152596 = -116856395;    double TXecJzrHSr24483711 = -822335420;    double TXecJzrHSr33276077 = -417475193;    double TXecJzrHSr49507019 = 93981986;    double TXecJzrHSr69540674 = -830217782;    double TXecJzrHSr47152782 = -164054396;    double TXecJzrHSr35344545 = -512774203;    double TXecJzrHSr51414932 = -924121054;    double TXecJzrHSr59747925 = -764875237;    double TXecJzrHSr42931142 = -484304372;    double TXecJzrHSr58664923 = -186010946;    double TXecJzrHSr83277454 = -877341083;    double TXecJzrHSr80271233 = -9010903;    double TXecJzrHSr84193676 = -839594353;    double TXecJzrHSr76089948 = -972080435;    double TXecJzrHSr76974485 = -598393075;    double TXecJzrHSr55353267 = 98259559;    double TXecJzrHSr67754062 = -903379013;    double TXecJzrHSr70241019 = -19400345;    double TXecJzrHSr59058520 = -924058123;    double TXecJzrHSr55016298 = -775789815;    double TXecJzrHSr87373342 = -545076033;     TXecJzrHSr71398169 = TXecJzrHSr39585995;     TXecJzrHSr39585995 = TXecJzrHSr26472520;     TXecJzrHSr26472520 = TXecJzrHSr74479563;     TXecJzrHSr74479563 = TXecJzrHSr65420754;     TXecJzrHSr65420754 = TXecJzrHSr10419753;     TXecJzrHSr10419753 = TXecJzrHSr94645586;     TXecJzrHSr94645586 = TXecJzrHSr62914801;     TXecJzrHSr62914801 = TXecJzrHSr90363675;     TXecJzrHSr90363675 = TXecJzrHSr41047821;     TXecJzrHSr41047821 = TXecJzrHSr93119182;     TXecJzrHSr93119182 = TXecJzrHSr70428089;     TXecJzrHSr70428089 = TXecJzrHSr69819632;     TXecJzrHSr69819632 = TXecJzrHSr99325602;     TXecJzrHSr99325602 = TXecJzrHSr3020622;     TXecJzrHSr3020622 = TXecJzrHSr66690426;     TXecJzrHSr66690426 = TXecJzrHSr52206463;     TXecJzrHSr52206463 = TXecJzrHSr79634280;     TXecJzrHSr79634280 = TXecJzrHSr91561875;     TXecJzrHSr91561875 = TXecJzrHSr17096116;     TXecJzrHSr17096116 = TXecJzrHSr64773925;     TXecJzrHSr64773925 = TXecJzrHSr33895082;     TXecJzrHSr33895082 = TXecJzrHSr63345245;     TXecJzrHSr63345245 = TXecJzrHSr95866956;     TXecJzrHSr95866956 = TXecJzrHSr78152125;     TXecJzrHSr78152125 = TXecJzrHSr59115077;     TXecJzrHSr59115077 = TXecJzrHSr93148225;     TXecJzrHSr93148225 = TXecJzrHSr32669281;     TXecJzrHSr32669281 = TXecJzrHSr38214255;     TXecJzrHSr38214255 = TXecJzrHSr84734569;     TXecJzrHSr84734569 = TXecJzrHSr78774469;     TXecJzrHSr78774469 = TXecJzrHSr45778164;     TXecJzrHSr45778164 = TXecJzrHSr58116400;     TXecJzrHSr58116400 = TXecJzrHSr47434044;     TXecJzrHSr47434044 = TXecJzrHSr20780542;     TXecJzrHSr20780542 = TXecJzrHSr56511983;     TXecJzrHSr56511983 = TXecJzrHSr14799240;     TXecJzrHSr14799240 = TXecJzrHSr72417385;     TXecJzrHSr72417385 = TXecJzrHSr83280162;     TXecJzrHSr83280162 = TXecJzrHSr72624794;     TXecJzrHSr72624794 = TXecJzrHSr29730368;     TXecJzrHSr29730368 = TXecJzrHSr94941565;     TXecJzrHSr94941565 = TXecJzrHSr64687626;     TXecJzrHSr64687626 = TXecJzrHSr17289650;     TXecJzrHSr17289650 = TXecJzrHSr34714360;     TXecJzrHSr34714360 = TXecJzrHSr53668414;     TXecJzrHSr53668414 = TXecJzrHSr25839000;     TXecJzrHSr25839000 = TXecJzrHSr43641207;     TXecJzrHSr43641207 = TXecJzrHSr63128607;     TXecJzrHSr63128607 = TXecJzrHSr91061472;     TXecJzrHSr91061472 = TXecJzrHSr49390025;     TXecJzrHSr49390025 = TXecJzrHSr27359537;     TXecJzrHSr27359537 = TXecJzrHSr86030239;     TXecJzrHSr86030239 = TXecJzrHSr15185259;     TXecJzrHSr15185259 = TXecJzrHSr88769120;     TXecJzrHSr88769120 = TXecJzrHSr37503087;     TXecJzrHSr37503087 = TXecJzrHSr76240749;     TXecJzrHSr76240749 = TXecJzrHSr30605563;     TXecJzrHSr30605563 = TXecJzrHSr96327437;     TXecJzrHSr96327437 = TXecJzrHSr6305677;     TXecJzrHSr6305677 = TXecJzrHSr17271527;     TXecJzrHSr17271527 = TXecJzrHSr61976305;     TXecJzrHSr61976305 = TXecJzrHSr24700546;     TXecJzrHSr24700546 = TXecJzrHSr5629107;     TXecJzrHSr5629107 = TXecJzrHSr62273352;     TXecJzrHSr62273352 = TXecJzrHSr47341018;     TXecJzrHSr47341018 = TXecJzrHSr12311690;     TXecJzrHSr12311690 = TXecJzrHSr22385588;     TXecJzrHSr22385588 = TXecJzrHSr78545060;     TXecJzrHSr78545060 = TXecJzrHSr46508639;     TXecJzrHSr46508639 = TXecJzrHSr51891187;     TXecJzrHSr51891187 = TXecJzrHSr79789077;     TXecJzrHSr79789077 = TXecJzrHSr96354117;     TXecJzrHSr96354117 = TXecJzrHSr18937082;     TXecJzrHSr18937082 = TXecJzrHSr87365748;     TXecJzrHSr87365748 = TXecJzrHSr69832360;     TXecJzrHSr69832360 = TXecJzrHSr69207456;     TXecJzrHSr69207456 = TXecJzrHSr46055596;     TXecJzrHSr46055596 = TXecJzrHSr61152596;     TXecJzrHSr61152596 = TXecJzrHSr24483711;     TXecJzrHSr24483711 = TXecJzrHSr33276077;     TXecJzrHSr33276077 = TXecJzrHSr49507019;     TXecJzrHSr49507019 = TXecJzrHSr69540674;     TXecJzrHSr69540674 = TXecJzrHSr47152782;     TXecJzrHSr47152782 = TXecJzrHSr35344545;     TXecJzrHSr35344545 = TXecJzrHSr51414932;     TXecJzrHSr51414932 = TXecJzrHSr59747925;     TXecJzrHSr59747925 = TXecJzrHSr42931142;     TXecJzrHSr42931142 = TXecJzrHSr58664923;     TXecJzrHSr58664923 = TXecJzrHSr83277454;     TXecJzrHSr83277454 = TXecJzrHSr80271233;     TXecJzrHSr80271233 = TXecJzrHSr84193676;     TXecJzrHSr84193676 = TXecJzrHSr76089948;     TXecJzrHSr76089948 = TXecJzrHSr76974485;     TXecJzrHSr76974485 = TXecJzrHSr55353267;     TXecJzrHSr55353267 = TXecJzrHSr67754062;     TXecJzrHSr67754062 = TXecJzrHSr70241019;     TXecJzrHSr70241019 = TXecJzrHSr59058520;     TXecJzrHSr59058520 = TXecJzrHSr55016298;     TXecJzrHSr55016298 = TXecJzrHSr87373342;     TXecJzrHSr87373342 = TXecJzrHSr71398169;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void GREVhfCAkD68922311() {     double IFaYOsWvGz22854408 = -91974480;    double IFaYOsWvGz74533971 = -134642560;    double IFaYOsWvGz83095268 = -6519312;    double IFaYOsWvGz15199263 = -822367397;    double IFaYOsWvGz68704674 = -785696809;    double IFaYOsWvGz22467990 = -69167786;    double IFaYOsWvGz2581185 = -200549161;    double IFaYOsWvGz52713639 = -852458262;    double IFaYOsWvGz49971925 = -128810754;    double IFaYOsWvGz96285008 = -478676717;    double IFaYOsWvGz94938782 = -560301140;    double IFaYOsWvGz53778095 = -661423783;    double IFaYOsWvGz24064350 = -894026622;    double IFaYOsWvGz43347069 = -375007048;    double IFaYOsWvGz13711085 = -942727790;    double IFaYOsWvGz92992832 = -930226041;    double IFaYOsWvGz2063462 = 54232085;    double IFaYOsWvGz32091379 = -146004397;    double IFaYOsWvGz72599386 = -964809632;    double IFaYOsWvGz52085488 = -404118600;    double IFaYOsWvGz98822155 = -134346653;    double IFaYOsWvGz47459783 = -488291464;    double IFaYOsWvGz99344602 = 82195406;    double IFaYOsWvGz22081070 = -356241033;    double IFaYOsWvGz54541849 = -698364978;    double IFaYOsWvGz59169882 = -38479855;    double IFaYOsWvGz59839661 = -605927218;    double IFaYOsWvGz27930932 = -353668123;    double IFaYOsWvGz77127280 = -788786188;    double IFaYOsWvGz35004667 = 13584270;    double IFaYOsWvGz52724168 = -756036892;    double IFaYOsWvGz97121032 = -578133752;    double IFaYOsWvGz85623894 = -636607695;    double IFaYOsWvGz47891553 = -136530895;    double IFaYOsWvGz30109076 = -765786915;    double IFaYOsWvGz80739544 = -717746943;    double IFaYOsWvGz33298055 = -274343032;    double IFaYOsWvGz63448039 = -310711906;    double IFaYOsWvGz41434364 = -136848854;    double IFaYOsWvGz8013230 = -701382745;    double IFaYOsWvGz67295858 = -816162996;    double IFaYOsWvGz76115024 = -937199838;    double IFaYOsWvGz12037782 = -533201576;    double IFaYOsWvGz54934889 = -665617924;    double IFaYOsWvGz70700814 = -837839903;    double IFaYOsWvGz50701975 = -177524297;    double IFaYOsWvGz54841655 = -310850972;    double IFaYOsWvGz3554420 = -494296084;    double IFaYOsWvGz49119959 = -706376921;    double IFaYOsWvGz79176886 = -570540049;    double IFaYOsWvGz23236533 = 64869569;    double IFaYOsWvGz63334364 = -748180276;    double IFaYOsWvGz77999042 = 32585394;    double IFaYOsWvGz63988652 = -370766863;    double IFaYOsWvGz70100156 = 36060359;    double IFaYOsWvGz75394624 = -603683016;    double IFaYOsWvGz75189369 = -116837967;    double IFaYOsWvGz61014199 = -650278279;    double IFaYOsWvGz60657414 = -24002419;    double IFaYOsWvGz9534792 = -647216954;    double IFaYOsWvGz62628329 = -463240569;    double IFaYOsWvGz74650253 = -846881039;    double IFaYOsWvGz75586358 = 36327926;    double IFaYOsWvGz14967258 = -42395025;    double IFaYOsWvGz43560840 = -722639826;    double IFaYOsWvGz97817749 = -982167388;    double IFaYOsWvGz68154200 = 75183912;    double IFaYOsWvGz76172797 = -657495727;    double IFaYOsWvGz13237994 = -609220134;    double IFaYOsWvGz32971541 = -124980848;    double IFaYOsWvGz59694777 = -555883009;    double IFaYOsWvGz38615423 = -635056009;    double IFaYOsWvGz90657015 = 90844457;    double IFaYOsWvGz64586156 = -163426888;    double IFaYOsWvGz84789629 = -587955605;    double IFaYOsWvGz22707131 = -197146815;    double IFaYOsWvGz35422001 = -955089889;    double IFaYOsWvGz44409713 = -252186670;    double IFaYOsWvGz51380256 = -518401130;    double IFaYOsWvGz3839874 = -420840682;    double IFaYOsWvGz4328228 = -727628884;    double IFaYOsWvGz56285241 = -11631135;    double IFaYOsWvGz78810972 = -647291203;    double IFaYOsWvGz97950394 = -118246140;    double IFaYOsWvGz11768134 = 48714701;    double IFaYOsWvGz89389804 = 92143384;    double IFaYOsWvGz19121991 = -510719147;    double IFaYOsWvGz21635243 = -165840832;    double IFaYOsWvGz77791396 = -72591255;    double IFaYOsWvGz54714451 = -62103899;    double IFaYOsWvGz5550175 = -500908976;    double IFaYOsWvGz72283855 = -624064753;    double IFaYOsWvGz2790625 = -186709488;    double IFaYOsWvGz31899572 = -489631900;    double IFaYOsWvGz45384901 = -138142177;    double IFaYOsWvGz92645605 = -969281957;    double IFaYOsWvGz528667 = -873527765;    double IFaYOsWvGz97070523 = -390806551;    double IFaYOsWvGz11374049 = -942978098;    double IFaYOsWvGz72883064 = -91974480;     IFaYOsWvGz22854408 = IFaYOsWvGz74533971;     IFaYOsWvGz74533971 = IFaYOsWvGz83095268;     IFaYOsWvGz83095268 = IFaYOsWvGz15199263;     IFaYOsWvGz15199263 = IFaYOsWvGz68704674;     IFaYOsWvGz68704674 = IFaYOsWvGz22467990;     IFaYOsWvGz22467990 = IFaYOsWvGz2581185;     IFaYOsWvGz2581185 = IFaYOsWvGz52713639;     IFaYOsWvGz52713639 = IFaYOsWvGz49971925;     IFaYOsWvGz49971925 = IFaYOsWvGz96285008;     IFaYOsWvGz96285008 = IFaYOsWvGz94938782;     IFaYOsWvGz94938782 = IFaYOsWvGz53778095;     IFaYOsWvGz53778095 = IFaYOsWvGz24064350;     IFaYOsWvGz24064350 = IFaYOsWvGz43347069;     IFaYOsWvGz43347069 = IFaYOsWvGz13711085;     IFaYOsWvGz13711085 = IFaYOsWvGz92992832;     IFaYOsWvGz92992832 = IFaYOsWvGz2063462;     IFaYOsWvGz2063462 = IFaYOsWvGz32091379;     IFaYOsWvGz32091379 = IFaYOsWvGz72599386;     IFaYOsWvGz72599386 = IFaYOsWvGz52085488;     IFaYOsWvGz52085488 = IFaYOsWvGz98822155;     IFaYOsWvGz98822155 = IFaYOsWvGz47459783;     IFaYOsWvGz47459783 = IFaYOsWvGz99344602;     IFaYOsWvGz99344602 = IFaYOsWvGz22081070;     IFaYOsWvGz22081070 = IFaYOsWvGz54541849;     IFaYOsWvGz54541849 = IFaYOsWvGz59169882;     IFaYOsWvGz59169882 = IFaYOsWvGz59839661;     IFaYOsWvGz59839661 = IFaYOsWvGz27930932;     IFaYOsWvGz27930932 = IFaYOsWvGz77127280;     IFaYOsWvGz77127280 = IFaYOsWvGz35004667;     IFaYOsWvGz35004667 = IFaYOsWvGz52724168;     IFaYOsWvGz52724168 = IFaYOsWvGz97121032;     IFaYOsWvGz97121032 = IFaYOsWvGz85623894;     IFaYOsWvGz85623894 = IFaYOsWvGz47891553;     IFaYOsWvGz47891553 = IFaYOsWvGz30109076;     IFaYOsWvGz30109076 = IFaYOsWvGz80739544;     IFaYOsWvGz80739544 = IFaYOsWvGz33298055;     IFaYOsWvGz33298055 = IFaYOsWvGz63448039;     IFaYOsWvGz63448039 = IFaYOsWvGz41434364;     IFaYOsWvGz41434364 = IFaYOsWvGz8013230;     IFaYOsWvGz8013230 = IFaYOsWvGz67295858;     IFaYOsWvGz67295858 = IFaYOsWvGz76115024;     IFaYOsWvGz76115024 = IFaYOsWvGz12037782;     IFaYOsWvGz12037782 = IFaYOsWvGz54934889;     IFaYOsWvGz54934889 = IFaYOsWvGz70700814;     IFaYOsWvGz70700814 = IFaYOsWvGz50701975;     IFaYOsWvGz50701975 = IFaYOsWvGz54841655;     IFaYOsWvGz54841655 = IFaYOsWvGz3554420;     IFaYOsWvGz3554420 = IFaYOsWvGz49119959;     IFaYOsWvGz49119959 = IFaYOsWvGz79176886;     IFaYOsWvGz79176886 = IFaYOsWvGz23236533;     IFaYOsWvGz23236533 = IFaYOsWvGz63334364;     IFaYOsWvGz63334364 = IFaYOsWvGz77999042;     IFaYOsWvGz77999042 = IFaYOsWvGz63988652;     IFaYOsWvGz63988652 = IFaYOsWvGz70100156;     IFaYOsWvGz70100156 = IFaYOsWvGz75394624;     IFaYOsWvGz75394624 = IFaYOsWvGz75189369;     IFaYOsWvGz75189369 = IFaYOsWvGz61014199;     IFaYOsWvGz61014199 = IFaYOsWvGz60657414;     IFaYOsWvGz60657414 = IFaYOsWvGz9534792;     IFaYOsWvGz9534792 = IFaYOsWvGz62628329;     IFaYOsWvGz62628329 = IFaYOsWvGz74650253;     IFaYOsWvGz74650253 = IFaYOsWvGz75586358;     IFaYOsWvGz75586358 = IFaYOsWvGz14967258;     IFaYOsWvGz14967258 = IFaYOsWvGz43560840;     IFaYOsWvGz43560840 = IFaYOsWvGz97817749;     IFaYOsWvGz97817749 = IFaYOsWvGz68154200;     IFaYOsWvGz68154200 = IFaYOsWvGz76172797;     IFaYOsWvGz76172797 = IFaYOsWvGz13237994;     IFaYOsWvGz13237994 = IFaYOsWvGz32971541;     IFaYOsWvGz32971541 = IFaYOsWvGz59694777;     IFaYOsWvGz59694777 = IFaYOsWvGz38615423;     IFaYOsWvGz38615423 = IFaYOsWvGz90657015;     IFaYOsWvGz90657015 = IFaYOsWvGz64586156;     IFaYOsWvGz64586156 = IFaYOsWvGz84789629;     IFaYOsWvGz84789629 = IFaYOsWvGz22707131;     IFaYOsWvGz22707131 = IFaYOsWvGz35422001;     IFaYOsWvGz35422001 = IFaYOsWvGz44409713;     IFaYOsWvGz44409713 = IFaYOsWvGz51380256;     IFaYOsWvGz51380256 = IFaYOsWvGz3839874;     IFaYOsWvGz3839874 = IFaYOsWvGz4328228;     IFaYOsWvGz4328228 = IFaYOsWvGz56285241;     IFaYOsWvGz56285241 = IFaYOsWvGz78810972;     IFaYOsWvGz78810972 = IFaYOsWvGz97950394;     IFaYOsWvGz97950394 = IFaYOsWvGz11768134;     IFaYOsWvGz11768134 = IFaYOsWvGz89389804;     IFaYOsWvGz89389804 = IFaYOsWvGz19121991;     IFaYOsWvGz19121991 = IFaYOsWvGz21635243;     IFaYOsWvGz21635243 = IFaYOsWvGz77791396;     IFaYOsWvGz77791396 = IFaYOsWvGz54714451;     IFaYOsWvGz54714451 = IFaYOsWvGz5550175;     IFaYOsWvGz5550175 = IFaYOsWvGz72283855;     IFaYOsWvGz72283855 = IFaYOsWvGz2790625;     IFaYOsWvGz2790625 = IFaYOsWvGz31899572;     IFaYOsWvGz31899572 = IFaYOsWvGz45384901;     IFaYOsWvGz45384901 = IFaYOsWvGz92645605;     IFaYOsWvGz92645605 = IFaYOsWvGz528667;     IFaYOsWvGz528667 = IFaYOsWvGz97070523;     IFaYOsWvGz97070523 = IFaYOsWvGz11374049;     IFaYOsWvGz11374049 = IFaYOsWvGz72883064;     IFaYOsWvGz72883064 = IFaYOsWvGz22854408;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void lKhbUuSMwt5897484() {     double VVINQZSCJG90580656 = -261375000;    double VVINQZSCJG50456169 = -646215926;    double VVINQZSCJG52445113 = -917917156;    double VVINQZSCJG43850871 = -643213347;    double VVINQZSCJG1267360 = 27326108;    double VVINQZSCJG57982142 = -600058140;    double VVINQZSCJG42183556 = -525180262;    double VVINQZSCJG68386444 = -526663991;    double VVINQZSCJG42100809 = -779799493;    double VVINQZSCJG2056122 = -679423652;    double VVINQZSCJG74189953 = -991041041;    double VVINQZSCJG61208008 = -461223350;    double VVINQZSCJG59108129 = -630862111;    double VVINQZSCJG46839610 = -535765279;    double VVINQZSCJG65394397 = -625796724;    double VVINQZSCJG22045756 = -914529670;    double VVINQZSCJG7907810 = -669062476;    double VVINQZSCJG93471741 = -931254242;    double VVINQZSCJG33088687 = -332082476;    double VVINQZSCJG85040978 = -923411359;    double VVINQZSCJG45200299 = -368217184;    double VVINQZSCJG38318678 = -236605553;    double VVINQZSCJG3046491 = 33900393;    double VVINQZSCJG90802596 = -2113596;    double VVINQZSCJG22772169 = -373704504;    double VVINQZSCJG22806007 = -386871222;    double VVINQZSCJG75160739 = -109469145;    double VVINQZSCJG15996907 = -743037704;    double VVINQZSCJG56800611 = -298868747;    double VVINQZSCJG25816307 = -391385002;    double VVINQZSCJG78722123 = -675573752;    double VVINQZSCJG8394471 = -110530340;    double VVINQZSCJG84459111 = -811377792;    double VVINQZSCJG12907185 = -657218668;    double VVINQZSCJG54192321 = -870994003;    double VVINQZSCJG77934060 = -864912469;    double VVINQZSCJG63974646 = -256517741;    double VVINQZSCJG74014580 = -299671037;    double VVINQZSCJG74286442 = -868627700;    double VVINQZSCJG76085442 = -788729861;    double VVINQZSCJG27949496 = -194734402;    double VVINQZSCJG13646419 = -281368525;    double VVINQZSCJG86871151 = -39357096;    double VVINQZSCJG4616508 = -746266350;    double VVINQZSCJG62718069 = -987725185;    double VVINQZSCJG43896624 = -103008448;    double VVINQZSCJG78363133 = -243484463;    double VVINQZSCJG109366 = -881940059;    double VVINQZSCJG19895487 = -320127291;    double VVINQZSCJG51464392 = -59565900;    double VVINQZSCJG70251580 = -211546213;    double VVINQZSCJG62350729 = 53920676;    double VVINQZSCJG33791 = -891464387;    double VVINQZSCJG77109710 = -182481039;    double VVINQZSCJG60697185 = -565634353;    double VVINQZSCJG52261978 = 75230552;    double VVINQZSCJG47409679 = -580116319;    double VVINQZSCJG61642517 = -815803560;    double VVINQZSCJG21078702 = -169508844;    double VVINQZSCJG78461353 = -585802670;    double VVINQZSCJG82821403 = -390588996;    double VVINQZSCJG26186649 = -782142558;    double VVINQZSCJG11585834 = -127795244;    double VVINQZSCJG16284503 = -288414491;    double VVINQZSCJG23333998 = 96150100;    double VVINQZSCJG65795482 = -780510701;    double VVINQZSCJG76748896 = -649845558;    double VVINQZSCJG46200945 = -973643443;    double VVINQZSCJG92647289 = -664771276;    double VVINQZSCJG87460336 = -760884255;    double VVINQZSCJG58071109 = -558011929;    double VVINQZSCJG33893229 = -269391440;    double VVINQZSCJG19185299 = 37373458;    double VVINQZSCJG57003244 = -543352616;    double VVINQZSCJG57091483 = -628676957;    double VVINQZSCJG31553881 = 13151340;    double VVINQZSCJG51447527 = -97248457;    double VVINQZSCJG98429982 = -219833257;    double VVINQZSCJG28084527 = -14388412;    double VVINQZSCJG78875545 = -170696056;    double VVINQZSCJG44442873 = -43386760;    double VVINQZSCJG75051373 = -227529087;    double VVINQZSCJG96101419 = -322910413;    double VVINQZSCJG5336219 = -139302847;    double VVINQZSCJG55564727 = -79838789;    double VVINQZSCJG16371395 = -629494429;    double VVINQZSCJG8360681 = -219065954;    double VVINQZSCJG7349402 = -528896754;    double VVINQZSCJG52210000 = 8415684;    double VVINQZSCJG1930344 = -846224555;    double VVINQZSCJG30524382 = -184796150;    double VVINQZSCJG2332130 = -440714181;    double VVINQZSCJG52935878 = -30162193;    double VVINQZSCJG95825089 = -182825030;    double VVINQZSCJG93264039 = -298140866;    double VVINQZSCJG1762847 = -412591844;    double VVINQZSCJG2060586 = -53573281;    double VVINQZSCJG70586649 = -750942605;    double VVINQZSCJG81282509 = -742416451;    double VVINQZSCJG96922587 = -261375000;     VVINQZSCJG90580656 = VVINQZSCJG50456169;     VVINQZSCJG50456169 = VVINQZSCJG52445113;     VVINQZSCJG52445113 = VVINQZSCJG43850871;     VVINQZSCJG43850871 = VVINQZSCJG1267360;     VVINQZSCJG1267360 = VVINQZSCJG57982142;     VVINQZSCJG57982142 = VVINQZSCJG42183556;     VVINQZSCJG42183556 = VVINQZSCJG68386444;     VVINQZSCJG68386444 = VVINQZSCJG42100809;     VVINQZSCJG42100809 = VVINQZSCJG2056122;     VVINQZSCJG2056122 = VVINQZSCJG74189953;     VVINQZSCJG74189953 = VVINQZSCJG61208008;     VVINQZSCJG61208008 = VVINQZSCJG59108129;     VVINQZSCJG59108129 = VVINQZSCJG46839610;     VVINQZSCJG46839610 = VVINQZSCJG65394397;     VVINQZSCJG65394397 = VVINQZSCJG22045756;     VVINQZSCJG22045756 = VVINQZSCJG7907810;     VVINQZSCJG7907810 = VVINQZSCJG93471741;     VVINQZSCJG93471741 = VVINQZSCJG33088687;     VVINQZSCJG33088687 = VVINQZSCJG85040978;     VVINQZSCJG85040978 = VVINQZSCJG45200299;     VVINQZSCJG45200299 = VVINQZSCJG38318678;     VVINQZSCJG38318678 = VVINQZSCJG3046491;     VVINQZSCJG3046491 = VVINQZSCJG90802596;     VVINQZSCJG90802596 = VVINQZSCJG22772169;     VVINQZSCJG22772169 = VVINQZSCJG22806007;     VVINQZSCJG22806007 = VVINQZSCJG75160739;     VVINQZSCJG75160739 = VVINQZSCJG15996907;     VVINQZSCJG15996907 = VVINQZSCJG56800611;     VVINQZSCJG56800611 = VVINQZSCJG25816307;     VVINQZSCJG25816307 = VVINQZSCJG78722123;     VVINQZSCJG78722123 = VVINQZSCJG8394471;     VVINQZSCJG8394471 = VVINQZSCJG84459111;     VVINQZSCJG84459111 = VVINQZSCJG12907185;     VVINQZSCJG12907185 = VVINQZSCJG54192321;     VVINQZSCJG54192321 = VVINQZSCJG77934060;     VVINQZSCJG77934060 = VVINQZSCJG63974646;     VVINQZSCJG63974646 = VVINQZSCJG74014580;     VVINQZSCJG74014580 = VVINQZSCJG74286442;     VVINQZSCJG74286442 = VVINQZSCJG76085442;     VVINQZSCJG76085442 = VVINQZSCJG27949496;     VVINQZSCJG27949496 = VVINQZSCJG13646419;     VVINQZSCJG13646419 = VVINQZSCJG86871151;     VVINQZSCJG86871151 = VVINQZSCJG4616508;     VVINQZSCJG4616508 = VVINQZSCJG62718069;     VVINQZSCJG62718069 = VVINQZSCJG43896624;     VVINQZSCJG43896624 = VVINQZSCJG78363133;     VVINQZSCJG78363133 = VVINQZSCJG109366;     VVINQZSCJG109366 = VVINQZSCJG19895487;     VVINQZSCJG19895487 = VVINQZSCJG51464392;     VVINQZSCJG51464392 = VVINQZSCJG70251580;     VVINQZSCJG70251580 = VVINQZSCJG62350729;     VVINQZSCJG62350729 = VVINQZSCJG33791;     VVINQZSCJG33791 = VVINQZSCJG77109710;     VVINQZSCJG77109710 = VVINQZSCJG60697185;     VVINQZSCJG60697185 = VVINQZSCJG52261978;     VVINQZSCJG52261978 = VVINQZSCJG47409679;     VVINQZSCJG47409679 = VVINQZSCJG61642517;     VVINQZSCJG61642517 = VVINQZSCJG21078702;     VVINQZSCJG21078702 = VVINQZSCJG78461353;     VVINQZSCJG78461353 = VVINQZSCJG82821403;     VVINQZSCJG82821403 = VVINQZSCJG26186649;     VVINQZSCJG26186649 = VVINQZSCJG11585834;     VVINQZSCJG11585834 = VVINQZSCJG16284503;     VVINQZSCJG16284503 = VVINQZSCJG23333998;     VVINQZSCJG23333998 = VVINQZSCJG65795482;     VVINQZSCJG65795482 = VVINQZSCJG76748896;     VVINQZSCJG76748896 = VVINQZSCJG46200945;     VVINQZSCJG46200945 = VVINQZSCJG92647289;     VVINQZSCJG92647289 = VVINQZSCJG87460336;     VVINQZSCJG87460336 = VVINQZSCJG58071109;     VVINQZSCJG58071109 = VVINQZSCJG33893229;     VVINQZSCJG33893229 = VVINQZSCJG19185299;     VVINQZSCJG19185299 = VVINQZSCJG57003244;     VVINQZSCJG57003244 = VVINQZSCJG57091483;     VVINQZSCJG57091483 = VVINQZSCJG31553881;     VVINQZSCJG31553881 = VVINQZSCJG51447527;     VVINQZSCJG51447527 = VVINQZSCJG98429982;     VVINQZSCJG98429982 = VVINQZSCJG28084527;     VVINQZSCJG28084527 = VVINQZSCJG78875545;     VVINQZSCJG78875545 = VVINQZSCJG44442873;     VVINQZSCJG44442873 = VVINQZSCJG75051373;     VVINQZSCJG75051373 = VVINQZSCJG96101419;     VVINQZSCJG96101419 = VVINQZSCJG5336219;     VVINQZSCJG5336219 = VVINQZSCJG55564727;     VVINQZSCJG55564727 = VVINQZSCJG16371395;     VVINQZSCJG16371395 = VVINQZSCJG8360681;     VVINQZSCJG8360681 = VVINQZSCJG7349402;     VVINQZSCJG7349402 = VVINQZSCJG52210000;     VVINQZSCJG52210000 = VVINQZSCJG1930344;     VVINQZSCJG1930344 = VVINQZSCJG30524382;     VVINQZSCJG30524382 = VVINQZSCJG2332130;     VVINQZSCJG2332130 = VVINQZSCJG52935878;     VVINQZSCJG52935878 = VVINQZSCJG95825089;     VVINQZSCJG95825089 = VVINQZSCJG93264039;     VVINQZSCJG93264039 = VVINQZSCJG1762847;     VVINQZSCJG1762847 = VVINQZSCJG2060586;     VVINQZSCJG2060586 = VVINQZSCJG70586649;     VVINQZSCJG70586649 = VVINQZSCJG81282509;     VVINQZSCJG81282509 = VVINQZSCJG96922587;     VVINQZSCJG96922587 = VVINQZSCJG90580656;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void DtZxrrKYel90630124() {     double hhmQhDblYH28965043 = -76400510;    double hhmQhDblYH56479567 = -986837466;    double hhmQhDblYH450665 = -114480847;    double hhmQhDblYH27353905 = -833656857;    double hhmQhDblYH46440265 = -674702001;    double hhmQhDblYH48690624 = -33096833;    double hhmQhDblYH31617487 = -44050606;    double hhmQhDblYH71687580 = -843509492;    double hhmQhDblYH97354584 = -710489490;    double hhmQhDblYH34154182 = -444459066;    double hhmQhDblYH75369678 = -397210799;    double hhmQhDblYH99892815 = -841308347;    double hhmQhDblYH42126382 = -255576185;    double hhmQhDblYH80334965 = -152398668;    double hhmQhDblYH54625620 = -495902808;    double hhmQhDblYH9062773 = -136021235;    double hhmQhDblYH51752224 = -381927597;    double hhmQhDblYH52145065 = -208598661;    double hhmQhDblYH54857498 = -888560996;    double hhmQhDblYH4709416 = -273045971;    double hhmQhDblYH67044772 = -800300482;    double hhmQhDblYH12277073 = -906213064;    double hhmQhDblYH71813791 = -123279923;    double hhmQhDblYH49723552 = -459466543;    double hhmQhDblYH26684188 = -398254864;    double hhmQhDblYH465635 = -925341106;    double hhmQhDblYH28521021 = -807389134;    double hhmQhDblYH76441581 = 71435935;    double hhmQhDblYH55937983 = -557239208;    double hhmQhDblYH79710173 = -579317971;    double hhmQhDblYH78379396 = 43548936;    double hhmQhDblYH79455512 = -679402330;    double hhmQhDblYH80787266 = -731165281;    double hhmQhDblYH43004954 = -864754393;    double hhmQhDblYH73941765 = -563833945;    double hhmQhDblYH92293646 = 88461204;    double hhmQhDblYH26501357 = -327632280;    double hhmQhDblYH67455931 = -77480595;    double hhmQhDblYH50358214 = -843453767;    double hhmQhDblYH67388060 = -784253654;    double hhmQhDblYH60820750 = -969380788;    double hhmQhDblYH93963142 = -301134199;    double hhmQhDblYH50388120 = -73136414;    double hhmQhDblYH1574528 = -241925305;    double hhmQhDblYH66671331 = -972997459;    double hhmQhDblYH3069139 = 25214916;    double hhmQhDblYH318023 = -910907131;    double hhmQhDblYH86894941 = -12952973;    double hhmQhDblYH77454077 = -355877770;    double hhmQhDblYH12133962 = -244575744;    double hhmQhDblYH72196581 = -455979109;    double hhmQhDblYH88691341 = -931052345;    double hhmQhDblYH1695848 = -751258202;    double hhmQhDblYH70207803 = -123156859;    double hhmQhDblYH23585352 = -421370701;    double hhmQhDblYH16687971 = -170187447;    double hhmQhDblYH84665776 = -763557544;    double hhmQhDblYH50727113 = -655014304;    double hhmQhDblYH669717 = -335401994;    double hhmQhDblYH45974630 = -749360895;    double hhmQhDblYH20169604 = -225707700;    double hhmQhDblYH55175905 = -15486541;    double hhmQhDblYH15749598 = -186270285;    double hhmQhDblYH17644411 = -31171519;    double hhmQhDblYH55774785 = -388008002;    double hhmQhDblYH95914166 = -717808470;    double hhmQhDblYH19105549 = -10143066;    double hhmQhDblYH99121428 = -390821792;    double hhmQhDblYH6393200 = -588564723;    double hhmQhDblYH62331973 = -484364012;    double hhmQhDblYH82561416 = -808388955;    double hhmQhDblYH84296293 = -204447003;    double hhmQhDblYH1786851 = -365144895;    double hhmQhDblYH87469437 = -4307342;    double hhmQhDblYH43888666 = -303665183;    double hhmQhDblYH73081630 = -399166284;    double hhmQhDblYH61888952 = -733076650;    double hhmQhDblYH70239263 = -881354618;    double hhmQhDblYH83052220 = -486469085;    double hhmQhDblYH23615050 = -323469781;    double hhmQhDblYH147613 = 85566025;    double hhmQhDblYH41626080 = -694436162;    double hhmQhDblYH98987504 = -572686295;    double hhmQhDblYH43804021 = -212663465;    double hhmQhDblYH7513593 = -23338862;    double hhmQhDblYH89688055 = -25398720;    double hhmQhDblYH77759664 = -928144128;    double hhmQhDblYH10579464 = -508008422;    double hhmQhDblYH19419603 = -343383692;    double hhmQhDblYH57253795 = -293646499;    double hhmQhDblYH7627870 = -147981253;    double hhmQhDblYH75774244 = -672617976;    double hhmQhDblYH66786214 = -742078602;    double hhmQhDblYH4383584 = 5907128;    double hhmQhDblYH47218457 = -458545955;    double hhmQhDblYH5644845 = -853894247;    double hhmQhDblYH78213545 = -14863914;    double hhmQhDblYH32743710 = 58035105;    double hhmQhDblYH45799743 = -853917304;    double hhmQhDblYH70757165 = -76400510;     hhmQhDblYH28965043 = hhmQhDblYH56479567;     hhmQhDblYH56479567 = hhmQhDblYH450665;     hhmQhDblYH450665 = hhmQhDblYH27353905;     hhmQhDblYH27353905 = hhmQhDblYH46440265;     hhmQhDblYH46440265 = hhmQhDblYH48690624;     hhmQhDblYH48690624 = hhmQhDblYH31617487;     hhmQhDblYH31617487 = hhmQhDblYH71687580;     hhmQhDblYH71687580 = hhmQhDblYH97354584;     hhmQhDblYH97354584 = hhmQhDblYH34154182;     hhmQhDblYH34154182 = hhmQhDblYH75369678;     hhmQhDblYH75369678 = hhmQhDblYH99892815;     hhmQhDblYH99892815 = hhmQhDblYH42126382;     hhmQhDblYH42126382 = hhmQhDblYH80334965;     hhmQhDblYH80334965 = hhmQhDblYH54625620;     hhmQhDblYH54625620 = hhmQhDblYH9062773;     hhmQhDblYH9062773 = hhmQhDblYH51752224;     hhmQhDblYH51752224 = hhmQhDblYH52145065;     hhmQhDblYH52145065 = hhmQhDblYH54857498;     hhmQhDblYH54857498 = hhmQhDblYH4709416;     hhmQhDblYH4709416 = hhmQhDblYH67044772;     hhmQhDblYH67044772 = hhmQhDblYH12277073;     hhmQhDblYH12277073 = hhmQhDblYH71813791;     hhmQhDblYH71813791 = hhmQhDblYH49723552;     hhmQhDblYH49723552 = hhmQhDblYH26684188;     hhmQhDblYH26684188 = hhmQhDblYH465635;     hhmQhDblYH465635 = hhmQhDblYH28521021;     hhmQhDblYH28521021 = hhmQhDblYH76441581;     hhmQhDblYH76441581 = hhmQhDblYH55937983;     hhmQhDblYH55937983 = hhmQhDblYH79710173;     hhmQhDblYH79710173 = hhmQhDblYH78379396;     hhmQhDblYH78379396 = hhmQhDblYH79455512;     hhmQhDblYH79455512 = hhmQhDblYH80787266;     hhmQhDblYH80787266 = hhmQhDblYH43004954;     hhmQhDblYH43004954 = hhmQhDblYH73941765;     hhmQhDblYH73941765 = hhmQhDblYH92293646;     hhmQhDblYH92293646 = hhmQhDblYH26501357;     hhmQhDblYH26501357 = hhmQhDblYH67455931;     hhmQhDblYH67455931 = hhmQhDblYH50358214;     hhmQhDblYH50358214 = hhmQhDblYH67388060;     hhmQhDblYH67388060 = hhmQhDblYH60820750;     hhmQhDblYH60820750 = hhmQhDblYH93963142;     hhmQhDblYH93963142 = hhmQhDblYH50388120;     hhmQhDblYH50388120 = hhmQhDblYH1574528;     hhmQhDblYH1574528 = hhmQhDblYH66671331;     hhmQhDblYH66671331 = hhmQhDblYH3069139;     hhmQhDblYH3069139 = hhmQhDblYH318023;     hhmQhDblYH318023 = hhmQhDblYH86894941;     hhmQhDblYH86894941 = hhmQhDblYH77454077;     hhmQhDblYH77454077 = hhmQhDblYH12133962;     hhmQhDblYH12133962 = hhmQhDblYH72196581;     hhmQhDblYH72196581 = hhmQhDblYH88691341;     hhmQhDblYH88691341 = hhmQhDblYH1695848;     hhmQhDblYH1695848 = hhmQhDblYH70207803;     hhmQhDblYH70207803 = hhmQhDblYH23585352;     hhmQhDblYH23585352 = hhmQhDblYH16687971;     hhmQhDblYH16687971 = hhmQhDblYH84665776;     hhmQhDblYH84665776 = hhmQhDblYH50727113;     hhmQhDblYH50727113 = hhmQhDblYH669717;     hhmQhDblYH669717 = hhmQhDblYH45974630;     hhmQhDblYH45974630 = hhmQhDblYH20169604;     hhmQhDblYH20169604 = hhmQhDblYH55175905;     hhmQhDblYH55175905 = hhmQhDblYH15749598;     hhmQhDblYH15749598 = hhmQhDblYH17644411;     hhmQhDblYH17644411 = hhmQhDblYH55774785;     hhmQhDblYH55774785 = hhmQhDblYH95914166;     hhmQhDblYH95914166 = hhmQhDblYH19105549;     hhmQhDblYH19105549 = hhmQhDblYH99121428;     hhmQhDblYH99121428 = hhmQhDblYH6393200;     hhmQhDblYH6393200 = hhmQhDblYH62331973;     hhmQhDblYH62331973 = hhmQhDblYH82561416;     hhmQhDblYH82561416 = hhmQhDblYH84296293;     hhmQhDblYH84296293 = hhmQhDblYH1786851;     hhmQhDblYH1786851 = hhmQhDblYH87469437;     hhmQhDblYH87469437 = hhmQhDblYH43888666;     hhmQhDblYH43888666 = hhmQhDblYH73081630;     hhmQhDblYH73081630 = hhmQhDblYH61888952;     hhmQhDblYH61888952 = hhmQhDblYH70239263;     hhmQhDblYH70239263 = hhmQhDblYH83052220;     hhmQhDblYH83052220 = hhmQhDblYH23615050;     hhmQhDblYH23615050 = hhmQhDblYH147613;     hhmQhDblYH147613 = hhmQhDblYH41626080;     hhmQhDblYH41626080 = hhmQhDblYH98987504;     hhmQhDblYH98987504 = hhmQhDblYH43804021;     hhmQhDblYH43804021 = hhmQhDblYH7513593;     hhmQhDblYH7513593 = hhmQhDblYH89688055;     hhmQhDblYH89688055 = hhmQhDblYH77759664;     hhmQhDblYH77759664 = hhmQhDblYH10579464;     hhmQhDblYH10579464 = hhmQhDblYH19419603;     hhmQhDblYH19419603 = hhmQhDblYH57253795;     hhmQhDblYH57253795 = hhmQhDblYH7627870;     hhmQhDblYH7627870 = hhmQhDblYH75774244;     hhmQhDblYH75774244 = hhmQhDblYH66786214;     hhmQhDblYH66786214 = hhmQhDblYH4383584;     hhmQhDblYH4383584 = hhmQhDblYH47218457;     hhmQhDblYH47218457 = hhmQhDblYH5644845;     hhmQhDblYH5644845 = hhmQhDblYH78213545;     hhmQhDblYH78213545 = hhmQhDblYH32743710;     hhmQhDblYH32743710 = hhmQhDblYH45799743;     hhmQhDblYH45799743 = hhmQhDblYH70757165;     hhmQhDblYH70757165 = hhmQhDblYH28965043;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void zqoBNEjSRP66642244() {     double vUKgClkWFQ15886721 = -217051715;    double vUKgClkWFQ66127306 = 2575961;    double vUKgClkWFQ93146678 = -645596229;    double vUKgClkWFQ55217099 = -839454148;    double vUKgClkWFQ97169351 = -528515479;    double vUKgClkWFQ62156302 = -430790128;    double vUKgClkWFQ81663155 = -825848644;    double vUKgClkWFQ54403929 = -274049313;    double vUKgClkWFQ75740274 = -920000192;    double vUKgClkWFQ7654568 = -159320272;    double vUKgClkWFQ95050408 = -551299543;    double vUKgClkWFQ69519292 = -963411231;    double vUKgClkWFQ2752832 = -343939474;    double vUKgClkWFQ23653075 = 51102933;    double vUKgClkWFQ78338489 = -682668358;    double vUKgClkWFQ44341932 = -114672821;    double vUKgClkWFQ1592400 = 48152566;    double vUKgClkWFQ81361822 = -983984905;    double vUKgClkWFQ16017069 = -225081966;    double vUKgClkWFQ61462243 = -711143810;    double vUKgClkWFQ34510441 = -844979476;    double vUKgClkWFQ50967032 = -734334966;    double vUKgClkWFQ41460131 = -20686173;    double vUKgClkWFQ26080502 = -393555319;    double vUKgClkWFQ82649173 = -422522643;    double vUKgClkWFQ94644534 = -488864452;    double vUKgClkWFQ85411448 = 40508801;    double vUKgClkWFQ77028132 = -156213332;    double vUKgClkWFQ28840776 = -973471839;    double vUKgClkWFQ59423812 = -140538041;    double vUKgClkWFQ37499649 = -972879964;    double vUKgClkWFQ83897542 = -850324032;    double vUKgClkWFQ35060349 = -214857015;    double vUKgClkWFQ72928052 = 99130838;    double vUKgClkWFQ66720714 = -935804042;    double vUKgClkWFQ25253862 = -92134613;    double vUKgClkWFQ82470620 = -295537569;    double vUKgClkWFQ39784308 = 42286835;    double vUKgClkWFQ3589381 = -106304939;    double vUKgClkWFQ95175135 = 35353177;    double vUKgClkWFQ43982181 = -691303438;    double vUKgClkWFQ78804068 = -658289681;    double vUKgClkWFQ26838294 = -966616466;    double vUKgClkWFQ20119208 = -291920988;    double vUKgClkWFQ18656192 = -328889177;    double vUKgClkWFQ5636061 = 69864782;    double vUKgClkWFQ20968049 = -386611646;    double vUKgClkWFQ89150885 = -211722726;    double vUKgClkWFQ67679705 = -205621449;    double vUKgClkWFQ4733542 = -582594073;    double vUKgClkWFQ13554444 = -515333836;    double vUKgClkWFQ93604383 = -430365028;    double vUKgClkWFQ86837450 = -559177886;    double vUKgClkWFQ24752773 = -739249020;    double vUKgClkWFQ32131803 = 27516052;    double vUKgClkWFQ64919689 = -482716749;    double vUKgClkWFQ24667175 = -976737867;    double vUKgClkWFQ67066176 = -152040911;    double vUKgClkWFQ72567925 = -316931505;    double vUKgClkWFQ2524818 = 60348973;    double vUKgClkWFQ76744853 = -371298929;    double vUKgClkWFQ4635024 = -569635313;    double vUKgClkWFQ25563153 = -300577474;    double vUKgClkWFQ16316463 = -679462152;    double vUKgClkWFQ70154919 = -186440309;    double vUKgClkWFQ11152867 = -700975512;    double vUKgClkWFQ34458944 = -648554217;    double vUKgClkWFQ29824779 = -343070312;    double vUKgClkWFQ56932360 = -13093025;    double vUKgClkWFQ53084628 = -490533746;    double vUKgClkWFQ61871312 = -819135252;    double vUKgClkWFQ61808092 = -994134270;    double vUKgClkWFQ77772442 = -777679967;    double vUKgClkWFQ20841934 = -160435143;    double vUKgClkWFQ17480063 = 80159628;    double vUKgClkWFQ55706373 = -86689795;    double vUKgClkWFQ24128738 = -767718501;    double vUKgClkWFQ21340924 = -728765185;    double vUKgClkWFQ7424311 = 35333858;    double vUKgClkWFQ77013113 = -392387426;    double vUKgClkWFQ73676486 = -2252806;    double vUKgClkWFQ96260563 = -747768473;    double vUKgClkWFQ9348427 = -950591883;    double vUKgClkWFQ24107235 = -290877767;    double vUKgClkWFQ45869368 = -625204205;    double vUKgClkWFQ43895265 = -442514936;    double vUKgClkWFQ97060091 = -191146146;    double vUKgClkWFQ10307577 = -475607995;    double vUKgClkWFQ40796249 = -928385214;    double vUKgClkWFQ1801026 = -353087293;    double vUKgClkWFQ586687 = -115396747;    double vUKgClkWFQ15404444 = -43496659;    double vUKgClkWFQ67216382 = -640781661;    double vUKgClkWFQ1064564 = -66653912;    double vUKgClkWFQ18430283 = -593347895;    double vUKgClkWFQ39347157 = -21668126;    double vUKgClkWFQ53240915 = -257712207;    double vUKgClkWFQ10521832 = -187154315;    double vUKgClkWFQ49964288 = -5480680;    double vUKgClkWFQ7503326 = -217051715;     vUKgClkWFQ15886721 = vUKgClkWFQ66127306;     vUKgClkWFQ66127306 = vUKgClkWFQ93146678;     vUKgClkWFQ93146678 = vUKgClkWFQ55217099;     vUKgClkWFQ55217099 = vUKgClkWFQ97169351;     vUKgClkWFQ97169351 = vUKgClkWFQ62156302;     vUKgClkWFQ62156302 = vUKgClkWFQ81663155;     vUKgClkWFQ81663155 = vUKgClkWFQ54403929;     vUKgClkWFQ54403929 = vUKgClkWFQ75740274;     vUKgClkWFQ75740274 = vUKgClkWFQ7654568;     vUKgClkWFQ7654568 = vUKgClkWFQ95050408;     vUKgClkWFQ95050408 = vUKgClkWFQ69519292;     vUKgClkWFQ69519292 = vUKgClkWFQ2752832;     vUKgClkWFQ2752832 = vUKgClkWFQ23653075;     vUKgClkWFQ23653075 = vUKgClkWFQ78338489;     vUKgClkWFQ78338489 = vUKgClkWFQ44341932;     vUKgClkWFQ44341932 = vUKgClkWFQ1592400;     vUKgClkWFQ1592400 = vUKgClkWFQ81361822;     vUKgClkWFQ81361822 = vUKgClkWFQ16017069;     vUKgClkWFQ16017069 = vUKgClkWFQ61462243;     vUKgClkWFQ61462243 = vUKgClkWFQ34510441;     vUKgClkWFQ34510441 = vUKgClkWFQ50967032;     vUKgClkWFQ50967032 = vUKgClkWFQ41460131;     vUKgClkWFQ41460131 = vUKgClkWFQ26080502;     vUKgClkWFQ26080502 = vUKgClkWFQ82649173;     vUKgClkWFQ82649173 = vUKgClkWFQ94644534;     vUKgClkWFQ94644534 = vUKgClkWFQ85411448;     vUKgClkWFQ85411448 = vUKgClkWFQ77028132;     vUKgClkWFQ77028132 = vUKgClkWFQ28840776;     vUKgClkWFQ28840776 = vUKgClkWFQ59423812;     vUKgClkWFQ59423812 = vUKgClkWFQ37499649;     vUKgClkWFQ37499649 = vUKgClkWFQ83897542;     vUKgClkWFQ83897542 = vUKgClkWFQ35060349;     vUKgClkWFQ35060349 = vUKgClkWFQ72928052;     vUKgClkWFQ72928052 = vUKgClkWFQ66720714;     vUKgClkWFQ66720714 = vUKgClkWFQ25253862;     vUKgClkWFQ25253862 = vUKgClkWFQ82470620;     vUKgClkWFQ82470620 = vUKgClkWFQ39784308;     vUKgClkWFQ39784308 = vUKgClkWFQ3589381;     vUKgClkWFQ3589381 = vUKgClkWFQ95175135;     vUKgClkWFQ95175135 = vUKgClkWFQ43982181;     vUKgClkWFQ43982181 = vUKgClkWFQ78804068;     vUKgClkWFQ78804068 = vUKgClkWFQ26838294;     vUKgClkWFQ26838294 = vUKgClkWFQ20119208;     vUKgClkWFQ20119208 = vUKgClkWFQ18656192;     vUKgClkWFQ18656192 = vUKgClkWFQ5636061;     vUKgClkWFQ5636061 = vUKgClkWFQ20968049;     vUKgClkWFQ20968049 = vUKgClkWFQ89150885;     vUKgClkWFQ89150885 = vUKgClkWFQ67679705;     vUKgClkWFQ67679705 = vUKgClkWFQ4733542;     vUKgClkWFQ4733542 = vUKgClkWFQ13554444;     vUKgClkWFQ13554444 = vUKgClkWFQ93604383;     vUKgClkWFQ93604383 = vUKgClkWFQ86837450;     vUKgClkWFQ86837450 = vUKgClkWFQ24752773;     vUKgClkWFQ24752773 = vUKgClkWFQ32131803;     vUKgClkWFQ32131803 = vUKgClkWFQ64919689;     vUKgClkWFQ64919689 = vUKgClkWFQ24667175;     vUKgClkWFQ24667175 = vUKgClkWFQ67066176;     vUKgClkWFQ67066176 = vUKgClkWFQ72567925;     vUKgClkWFQ72567925 = vUKgClkWFQ2524818;     vUKgClkWFQ2524818 = vUKgClkWFQ76744853;     vUKgClkWFQ76744853 = vUKgClkWFQ4635024;     vUKgClkWFQ4635024 = vUKgClkWFQ25563153;     vUKgClkWFQ25563153 = vUKgClkWFQ16316463;     vUKgClkWFQ16316463 = vUKgClkWFQ70154919;     vUKgClkWFQ70154919 = vUKgClkWFQ11152867;     vUKgClkWFQ11152867 = vUKgClkWFQ34458944;     vUKgClkWFQ34458944 = vUKgClkWFQ29824779;     vUKgClkWFQ29824779 = vUKgClkWFQ56932360;     vUKgClkWFQ56932360 = vUKgClkWFQ53084628;     vUKgClkWFQ53084628 = vUKgClkWFQ61871312;     vUKgClkWFQ61871312 = vUKgClkWFQ61808092;     vUKgClkWFQ61808092 = vUKgClkWFQ77772442;     vUKgClkWFQ77772442 = vUKgClkWFQ20841934;     vUKgClkWFQ20841934 = vUKgClkWFQ17480063;     vUKgClkWFQ17480063 = vUKgClkWFQ55706373;     vUKgClkWFQ55706373 = vUKgClkWFQ24128738;     vUKgClkWFQ24128738 = vUKgClkWFQ21340924;     vUKgClkWFQ21340924 = vUKgClkWFQ7424311;     vUKgClkWFQ7424311 = vUKgClkWFQ77013113;     vUKgClkWFQ77013113 = vUKgClkWFQ73676486;     vUKgClkWFQ73676486 = vUKgClkWFQ96260563;     vUKgClkWFQ96260563 = vUKgClkWFQ9348427;     vUKgClkWFQ9348427 = vUKgClkWFQ24107235;     vUKgClkWFQ24107235 = vUKgClkWFQ45869368;     vUKgClkWFQ45869368 = vUKgClkWFQ43895265;     vUKgClkWFQ43895265 = vUKgClkWFQ97060091;     vUKgClkWFQ97060091 = vUKgClkWFQ10307577;     vUKgClkWFQ10307577 = vUKgClkWFQ40796249;     vUKgClkWFQ40796249 = vUKgClkWFQ1801026;     vUKgClkWFQ1801026 = vUKgClkWFQ586687;     vUKgClkWFQ586687 = vUKgClkWFQ15404444;     vUKgClkWFQ15404444 = vUKgClkWFQ67216382;     vUKgClkWFQ67216382 = vUKgClkWFQ1064564;     vUKgClkWFQ1064564 = vUKgClkWFQ18430283;     vUKgClkWFQ18430283 = vUKgClkWFQ39347157;     vUKgClkWFQ39347157 = vUKgClkWFQ53240915;     vUKgClkWFQ53240915 = vUKgClkWFQ10521832;     vUKgClkWFQ10521832 = vUKgClkWFQ49964288;     vUKgClkWFQ49964288 = vUKgClkWFQ7503326;     vUKgClkWFQ7503326 = vUKgClkWFQ15886721;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void LurVeZvKmN12337937() {     double NGnizmNaeA35075678 = -60826540;    double NGnizmNaeA38425163 = -739032372;    double NGnizmNaeA17806061 = -222442382;    double NGnizmNaeA39508546 = -844946318;    double NGnizmNaeA24175856 = -563707194;    double NGnizmNaeA74913259 = 2974120;    double NGnizmNaeA60653788 = -987552050;    double NGnizmNaeA90661521 = -834560722;    double NGnizmNaeA44737244 = -192168226;    double NGnizmNaeA72023354 = -410241414;    double NGnizmNaeA55800574 = -234120458;    double NGnizmNaeA46007535 = 78807089;    double NGnizmNaeA60188415 = -717125748;    double NGnizmNaeA17322863 = 70209713;    double NGnizmNaeA95540155 = -49077827;    double NGnizmNaeA25132714 = -441816429;    double NGnizmNaeA1440987 = -818087280;    double NGnizmNaeA72198750 = -271192926;    double NGnizmNaeA37115610 = -812312359;    double NGnizmNaeA57333343 = -141973341;    double NGnizmNaeA35267390 = -366254312;    double NGnizmNaeA77094362 = -224134663;    double NGnizmNaeA44282979 = -328755251;    double NGnizmNaeA77366034 = -562692054;    double NGnizmNaeA98826527 = -98144750;    double NGnizmNaeA41761387 = -712202358;    double NGnizmNaeA97202380 = 91148950;    double NGnizmNaeA24952232 = -603460007;    double NGnizmNaeA34748685 = -325692227;    double NGnizmNaeA24415680 = -72220212;    double NGnizmNaeA4034625 = -256865237;    double NGnizmNaeA61789991 = -780670907;    double NGnizmNaeA75950638 = -825722867;    double NGnizmNaeA38118356 = -492977891;    double NGnizmNaeA17774456 = -361880975;    double NGnizmNaeA3847750 = -205330650;    double NGnizmNaeA19704660 = -380921528;    double NGnizmNaeA71463823 = -944249284;    double NGnizmNaeA59282064 = -450058680;    double NGnizmNaeA26762891 = -867124563;    double NGnizmNaeA54345641 = -22598581;    double NGnizmNaeA11811261 = -765068559;    double NGnizmNaeA88738458 = -713071252;    double NGnizmNaeA48214167 = -918232687;    double NGnizmNaeA62641849 = -8155015;    double NGnizmNaeA55436302 = -872045871;    double NGnizmNaeA45794389 = -410963291;    double NGnizmNaeA70235463 = -631609861;    double NGnizmNaeA5788196 = -5378619;    double NGnizmNaeA45091037 = 81388562;    double NGnizmNaeA21156630 = -976827788;    double NGnizmNaeA14048319 = -13924413;    double NGnizmNaeA25392653 = -435101798;    double NGnizmNaeA76426954 = -975546856;    double NGnizmNaeA77070546 = -878801761;    double NGnizmNaeA57981316 = -836691877;    double NGnizmNaeA94142184 = -310277121;    double NGnizmNaeA40440026 = -659750328;    double NGnizmNaeA40682018 = -646801568;    double NGnizmNaeA82414468 = -851504837;    double NGnizmNaeA77710878 = 11825170;    double NGnizmNaeA35701557 = -284092043;    double NGnizmNaeA55912836 = -408868496;    double NGnizmNaeA20321564 = -19948014;    double NGnizmNaeA67988730 = -53376178;    double NGnizmNaeA94010582 = -453449551;    double NGnizmNaeA70056896 = -95470045;    double NGnizmNaeA22070059 = -124147858;    double NGnizmNaeA99548406 = -567909312;    double NGnizmNaeA91692406 = -843747177;    double NGnizmNaeA5428055 = 39105099;    double NGnizmNaeA29977164 = -873837997;    double NGnizmNaeA12916687 = -821134246;    double NGnizmNaeA10352719 = -945187797;    double NGnizmNaeA2987702 = -19374761;    double NGnizmNaeA23456130 = -601185753;    double NGnizmNaeA88355903 = -511063412;    double NGnizmNaeA96068812 = -410522565;    double NGnizmNaeA14724186 = -454537039;    double NGnizmNaeA43390226 = -226098880;    double NGnizmNaeA95966998 = -201239067;    double NGnizmNaeA26966918 = -277241189;    double NGnizmNaeA19164037 = -498081388;    double NGnizmNaeA89657648 = -307080790;    double NGnizmNaeA3259051 = -95392425;    double NGnizmNaeA89986306 = -142940824;    double NGnizmNaeA36397338 = -245569110;    double NGnizmNaeA99523683 = -850176012;    double NGnizmNaeA61047809 = -614176130;    double NGnizmNaeA59793139 = -525189098;    double NGnizmNaeA9705566 = -895053530;    double NGnizmNaeA79264633 = -721171200;    double NGnizmNaeA30781805 = -197447716;    double NGnizmNaeA76867596 = -598553844;    double NGnizmNaeA49052012 = -778949733;    double NGnizmNaeA18644085 = -738506538;    double NGnizmNaeA55898424 = -256200064;    double NGnizmNaeA68416895 = -593123239;    double NGnizmNaeA80225436 = -764856509;    double NGnizmNaeA68631266 = -60826540;     NGnizmNaeA35075678 = NGnizmNaeA38425163;     NGnizmNaeA38425163 = NGnizmNaeA17806061;     NGnizmNaeA17806061 = NGnizmNaeA39508546;     NGnizmNaeA39508546 = NGnizmNaeA24175856;     NGnizmNaeA24175856 = NGnizmNaeA74913259;     NGnizmNaeA74913259 = NGnizmNaeA60653788;     NGnizmNaeA60653788 = NGnizmNaeA90661521;     NGnizmNaeA90661521 = NGnizmNaeA44737244;     NGnizmNaeA44737244 = NGnizmNaeA72023354;     NGnizmNaeA72023354 = NGnizmNaeA55800574;     NGnizmNaeA55800574 = NGnizmNaeA46007535;     NGnizmNaeA46007535 = NGnizmNaeA60188415;     NGnizmNaeA60188415 = NGnizmNaeA17322863;     NGnizmNaeA17322863 = NGnizmNaeA95540155;     NGnizmNaeA95540155 = NGnizmNaeA25132714;     NGnizmNaeA25132714 = NGnizmNaeA1440987;     NGnizmNaeA1440987 = NGnizmNaeA72198750;     NGnizmNaeA72198750 = NGnizmNaeA37115610;     NGnizmNaeA37115610 = NGnizmNaeA57333343;     NGnizmNaeA57333343 = NGnizmNaeA35267390;     NGnizmNaeA35267390 = NGnizmNaeA77094362;     NGnizmNaeA77094362 = NGnizmNaeA44282979;     NGnizmNaeA44282979 = NGnizmNaeA77366034;     NGnizmNaeA77366034 = NGnizmNaeA98826527;     NGnizmNaeA98826527 = NGnizmNaeA41761387;     NGnizmNaeA41761387 = NGnizmNaeA97202380;     NGnizmNaeA97202380 = NGnizmNaeA24952232;     NGnizmNaeA24952232 = NGnizmNaeA34748685;     NGnizmNaeA34748685 = NGnizmNaeA24415680;     NGnizmNaeA24415680 = NGnizmNaeA4034625;     NGnizmNaeA4034625 = NGnizmNaeA61789991;     NGnizmNaeA61789991 = NGnizmNaeA75950638;     NGnizmNaeA75950638 = NGnizmNaeA38118356;     NGnizmNaeA38118356 = NGnizmNaeA17774456;     NGnizmNaeA17774456 = NGnizmNaeA3847750;     NGnizmNaeA3847750 = NGnizmNaeA19704660;     NGnizmNaeA19704660 = NGnizmNaeA71463823;     NGnizmNaeA71463823 = NGnizmNaeA59282064;     NGnizmNaeA59282064 = NGnizmNaeA26762891;     NGnizmNaeA26762891 = NGnizmNaeA54345641;     NGnizmNaeA54345641 = NGnizmNaeA11811261;     NGnizmNaeA11811261 = NGnizmNaeA88738458;     NGnizmNaeA88738458 = NGnizmNaeA48214167;     NGnizmNaeA48214167 = NGnizmNaeA62641849;     NGnizmNaeA62641849 = NGnizmNaeA55436302;     NGnizmNaeA55436302 = NGnizmNaeA45794389;     NGnizmNaeA45794389 = NGnizmNaeA70235463;     NGnizmNaeA70235463 = NGnizmNaeA5788196;     NGnizmNaeA5788196 = NGnizmNaeA45091037;     NGnizmNaeA45091037 = NGnizmNaeA21156630;     NGnizmNaeA21156630 = NGnizmNaeA14048319;     NGnizmNaeA14048319 = NGnizmNaeA25392653;     NGnizmNaeA25392653 = NGnizmNaeA76426954;     NGnizmNaeA76426954 = NGnizmNaeA77070546;     NGnizmNaeA77070546 = NGnizmNaeA57981316;     NGnizmNaeA57981316 = NGnizmNaeA94142184;     NGnizmNaeA94142184 = NGnizmNaeA40440026;     NGnizmNaeA40440026 = NGnizmNaeA40682018;     NGnizmNaeA40682018 = NGnizmNaeA82414468;     NGnizmNaeA82414468 = NGnizmNaeA77710878;     NGnizmNaeA77710878 = NGnizmNaeA35701557;     NGnizmNaeA35701557 = NGnizmNaeA55912836;     NGnizmNaeA55912836 = NGnizmNaeA20321564;     NGnizmNaeA20321564 = NGnizmNaeA67988730;     NGnizmNaeA67988730 = NGnizmNaeA94010582;     NGnizmNaeA94010582 = NGnizmNaeA70056896;     NGnizmNaeA70056896 = NGnizmNaeA22070059;     NGnizmNaeA22070059 = NGnizmNaeA99548406;     NGnizmNaeA99548406 = NGnizmNaeA91692406;     NGnizmNaeA91692406 = NGnizmNaeA5428055;     NGnizmNaeA5428055 = NGnizmNaeA29977164;     NGnizmNaeA29977164 = NGnizmNaeA12916687;     NGnizmNaeA12916687 = NGnizmNaeA10352719;     NGnizmNaeA10352719 = NGnizmNaeA2987702;     NGnizmNaeA2987702 = NGnizmNaeA23456130;     NGnizmNaeA23456130 = NGnizmNaeA88355903;     NGnizmNaeA88355903 = NGnizmNaeA96068812;     NGnizmNaeA96068812 = NGnizmNaeA14724186;     NGnizmNaeA14724186 = NGnizmNaeA43390226;     NGnizmNaeA43390226 = NGnizmNaeA95966998;     NGnizmNaeA95966998 = NGnizmNaeA26966918;     NGnizmNaeA26966918 = NGnizmNaeA19164037;     NGnizmNaeA19164037 = NGnizmNaeA89657648;     NGnizmNaeA89657648 = NGnizmNaeA3259051;     NGnizmNaeA3259051 = NGnizmNaeA89986306;     NGnizmNaeA89986306 = NGnizmNaeA36397338;     NGnizmNaeA36397338 = NGnizmNaeA99523683;     NGnizmNaeA99523683 = NGnizmNaeA61047809;     NGnizmNaeA61047809 = NGnizmNaeA59793139;     NGnizmNaeA59793139 = NGnizmNaeA9705566;     NGnizmNaeA9705566 = NGnizmNaeA79264633;     NGnizmNaeA79264633 = NGnizmNaeA30781805;     NGnizmNaeA30781805 = NGnizmNaeA76867596;     NGnizmNaeA76867596 = NGnizmNaeA49052012;     NGnizmNaeA49052012 = NGnizmNaeA18644085;     NGnizmNaeA18644085 = NGnizmNaeA55898424;     NGnizmNaeA55898424 = NGnizmNaeA68416895;     NGnizmNaeA68416895 = NGnizmNaeA80225436;     NGnizmNaeA80225436 = NGnizmNaeA68631266;     NGnizmNaeA68631266 = NGnizmNaeA35075678;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void RLrzZCGuvS49313109() {     double SJrNYrtiKK2801927 = -230227061;    double SJrNYrtiKK14347361 = -150605737;    double SJrNYrtiKK87155904 = -33840226;    double SJrNYrtiKK68160154 = -665792268;    double SJrNYrtiKK56738541 = -850684277;    double SJrNYrtiKK10427412 = -527916234;    double SJrNYrtiKK256160 = -212183150;    double SJrNYrtiKK6334328 = -508766451;    double SJrNYrtiKK36866129 = -843156964;    double SJrNYrtiKK77794467 = -610988349;    double SJrNYrtiKK35051744 = -664860358;    double SJrNYrtiKK53437447 = -820992478;    double SJrNYrtiKK95232194 = -453961238;    double SJrNYrtiKK20815404 = -90548517;    double SJrNYrtiKK47223468 = -832146760;    double SJrNYrtiKK54185638 = -426120057;    double SJrNYrtiKK7285335 = -441381841;    double SJrNYrtiKK33579113 = 43557229;    double SJrNYrtiKK97604909 = -179585203;    double SJrNYrtiKK90288834 = -661266100;    double SJrNYrtiKK81645533 = -600124843;    double SJrNYrtiKK67953257 = 27551248;    double SJrNYrtiKK47984867 = -377050265;    double SJrNYrtiKK46087561 = -208564617;    double SJrNYrtiKK67056848 = -873484276;    double SJrNYrtiKK5397512 = 39406274;    double SJrNYrtiKK12523459 = -512392977;    double SJrNYrtiKK13018207 = -992829588;    double SJrNYrtiKK14422016 = -935774786;    double SJrNYrtiKK15227321 = -477189484;    double SJrNYrtiKK30032580 = -176402098;    double SJrNYrtiKK73063429 = -313067495;    double SJrNYrtiKK74785855 = 99507036;    double SJrNYrtiKK3133988 = 86334336;    double SJrNYrtiKK41857701 = -467088063;    double SJrNYrtiKK1042267 = -352496176;    double SJrNYrtiKK50381251 = -363096237;    double SJrNYrtiKK82030364 = -933208414;    double SJrNYrtiKK92134143 = -81837526;    double SJrNYrtiKK94835103 = -954471679;    double SJrNYrtiKK14999279 = -501169987;    double SJrNYrtiKK49342654 = -109237246;    double SJrNYrtiKK63571828 = -219226772;    double SJrNYrtiKK97895784 = -998881113;    double SJrNYrtiKK54659104 = -158040296;    double SJrNYrtiKK48630950 = -797530023;    double SJrNYrtiKK69315867 = -343596783;    double SJrNYrtiKK66790409 = 80746164;    double SJrNYrtiKK76563723 = -719128990;    double SJrNYrtiKK17378544 = -507637289;    double SJrNYrtiKK68171676 = -153243570;    double SJrNYrtiKK13064683 = -311823460;    double SJrNYrtiKK47427402 = -259151579;    double SJrNYrtiKK89548013 = -787261031;    double SJrNYrtiKK67667575 = -380496473;    double SJrNYrtiKK34848670 = -157778309;    double SJrNYrtiKK66362493 = -773555473;    double SJrNYrtiKK41068344 = -825275609;    double SJrNYrtiKK1103306 = -792307993;    double SJrNYrtiKK51341030 = -790090552;    double SJrNYrtiKK97903952 = 84476743;    double SJrNYrtiKK87237952 = -219353563;    double SJrNYrtiKK91912311 = -572991666;    double SJrNYrtiKK21638808 = -265967480;    double SJrNYrtiKK47761888 = -334586252;    double SJrNYrtiKK61988315 = -251792864;    double SJrNYrtiKK78651592 = -820499515;    double SJrNYrtiKK92098206 = -440295574;    double SJrNYrtiKK78957702 = -623460455;    double SJrNYrtiKK46181202 = -379650584;    double SJrNYrtiKK3804387 = 36976179;    double SJrNYrtiKK25254970 = -508173428;    double SJrNYrtiKK41444969 = -874605245;    double SJrNYrtiKK2769807 = -225113525;    double SJrNYrtiKK75289555 = -60096113;    double SJrNYrtiKK32302880 = -390887597;    double SJrNYrtiKK4381430 = -753221980;    double SJrNYrtiKK50089082 = -378169152;    double SJrNYrtiKK91428456 = 49475679;    double SJrNYrtiKK18425898 = 24045746;    double SJrNYrtiKK36081644 = -616996943;    double SJrNYrtiKK45733049 = -493139141;    double SJrNYrtiKK36454484 = -173700599;    double SJrNYrtiKK97043472 = -328137497;    double SJrNYrtiKK47055644 = -223945914;    double SJrNYrtiKK16967897 = -864578638;    double SJrNYrtiKK25636028 = 46084083;    double SJrNYrtiKK85237842 = -113231933;    double SJrNYrtiKK35466413 = -533169191;    double SJrNYrtiKK7009032 = -209309754;    double SJrNYrtiKK34679773 = -578940703;    double SJrNYrtiKK9312908 = -537820628;    double SJrNYrtiKK80927059 = -40900421;    double SJrNYrtiKK40793114 = -291746974;    double SJrNYrtiKK96931150 = -938948422;    double SJrNYrtiKK27761327 = -181816425;    double SJrNYrtiKK57430343 = -536245580;    double SJrNYrtiKK41933020 = -953259292;    double SJrNYrtiKK50133897 = -564294862;    double SJrNYrtiKK92670789 = -230227061;     SJrNYrtiKK2801927 = SJrNYrtiKK14347361;     SJrNYrtiKK14347361 = SJrNYrtiKK87155904;     SJrNYrtiKK87155904 = SJrNYrtiKK68160154;     SJrNYrtiKK68160154 = SJrNYrtiKK56738541;     SJrNYrtiKK56738541 = SJrNYrtiKK10427412;     SJrNYrtiKK10427412 = SJrNYrtiKK256160;     SJrNYrtiKK256160 = SJrNYrtiKK6334328;     SJrNYrtiKK6334328 = SJrNYrtiKK36866129;     SJrNYrtiKK36866129 = SJrNYrtiKK77794467;     SJrNYrtiKK77794467 = SJrNYrtiKK35051744;     SJrNYrtiKK35051744 = SJrNYrtiKK53437447;     SJrNYrtiKK53437447 = SJrNYrtiKK95232194;     SJrNYrtiKK95232194 = SJrNYrtiKK20815404;     SJrNYrtiKK20815404 = SJrNYrtiKK47223468;     SJrNYrtiKK47223468 = SJrNYrtiKK54185638;     SJrNYrtiKK54185638 = SJrNYrtiKK7285335;     SJrNYrtiKK7285335 = SJrNYrtiKK33579113;     SJrNYrtiKK33579113 = SJrNYrtiKK97604909;     SJrNYrtiKK97604909 = SJrNYrtiKK90288834;     SJrNYrtiKK90288834 = SJrNYrtiKK81645533;     SJrNYrtiKK81645533 = SJrNYrtiKK67953257;     SJrNYrtiKK67953257 = SJrNYrtiKK47984867;     SJrNYrtiKK47984867 = SJrNYrtiKK46087561;     SJrNYrtiKK46087561 = SJrNYrtiKK67056848;     SJrNYrtiKK67056848 = SJrNYrtiKK5397512;     SJrNYrtiKK5397512 = SJrNYrtiKK12523459;     SJrNYrtiKK12523459 = SJrNYrtiKK13018207;     SJrNYrtiKK13018207 = SJrNYrtiKK14422016;     SJrNYrtiKK14422016 = SJrNYrtiKK15227321;     SJrNYrtiKK15227321 = SJrNYrtiKK30032580;     SJrNYrtiKK30032580 = SJrNYrtiKK73063429;     SJrNYrtiKK73063429 = SJrNYrtiKK74785855;     SJrNYrtiKK74785855 = SJrNYrtiKK3133988;     SJrNYrtiKK3133988 = SJrNYrtiKK41857701;     SJrNYrtiKK41857701 = SJrNYrtiKK1042267;     SJrNYrtiKK1042267 = SJrNYrtiKK50381251;     SJrNYrtiKK50381251 = SJrNYrtiKK82030364;     SJrNYrtiKK82030364 = SJrNYrtiKK92134143;     SJrNYrtiKK92134143 = SJrNYrtiKK94835103;     SJrNYrtiKK94835103 = SJrNYrtiKK14999279;     SJrNYrtiKK14999279 = SJrNYrtiKK49342654;     SJrNYrtiKK49342654 = SJrNYrtiKK63571828;     SJrNYrtiKK63571828 = SJrNYrtiKK97895784;     SJrNYrtiKK97895784 = SJrNYrtiKK54659104;     SJrNYrtiKK54659104 = SJrNYrtiKK48630950;     SJrNYrtiKK48630950 = SJrNYrtiKK69315867;     SJrNYrtiKK69315867 = SJrNYrtiKK66790409;     SJrNYrtiKK66790409 = SJrNYrtiKK76563723;     SJrNYrtiKK76563723 = SJrNYrtiKK17378544;     SJrNYrtiKK17378544 = SJrNYrtiKK68171676;     SJrNYrtiKK68171676 = SJrNYrtiKK13064683;     SJrNYrtiKK13064683 = SJrNYrtiKK47427402;     SJrNYrtiKK47427402 = SJrNYrtiKK89548013;     SJrNYrtiKK89548013 = SJrNYrtiKK67667575;     SJrNYrtiKK67667575 = SJrNYrtiKK34848670;     SJrNYrtiKK34848670 = SJrNYrtiKK66362493;     SJrNYrtiKK66362493 = SJrNYrtiKK41068344;     SJrNYrtiKK41068344 = SJrNYrtiKK1103306;     SJrNYrtiKK1103306 = SJrNYrtiKK51341030;     SJrNYrtiKK51341030 = SJrNYrtiKK97903952;     SJrNYrtiKK97903952 = SJrNYrtiKK87237952;     SJrNYrtiKK87237952 = SJrNYrtiKK91912311;     SJrNYrtiKK91912311 = SJrNYrtiKK21638808;     SJrNYrtiKK21638808 = SJrNYrtiKK47761888;     SJrNYrtiKK47761888 = SJrNYrtiKK61988315;     SJrNYrtiKK61988315 = SJrNYrtiKK78651592;     SJrNYrtiKK78651592 = SJrNYrtiKK92098206;     SJrNYrtiKK92098206 = SJrNYrtiKK78957702;     SJrNYrtiKK78957702 = SJrNYrtiKK46181202;     SJrNYrtiKK46181202 = SJrNYrtiKK3804387;     SJrNYrtiKK3804387 = SJrNYrtiKK25254970;     SJrNYrtiKK25254970 = SJrNYrtiKK41444969;     SJrNYrtiKK41444969 = SJrNYrtiKK2769807;     SJrNYrtiKK2769807 = SJrNYrtiKK75289555;     SJrNYrtiKK75289555 = SJrNYrtiKK32302880;     SJrNYrtiKK32302880 = SJrNYrtiKK4381430;     SJrNYrtiKK4381430 = SJrNYrtiKK50089082;     SJrNYrtiKK50089082 = SJrNYrtiKK91428456;     SJrNYrtiKK91428456 = SJrNYrtiKK18425898;     SJrNYrtiKK18425898 = SJrNYrtiKK36081644;     SJrNYrtiKK36081644 = SJrNYrtiKK45733049;     SJrNYrtiKK45733049 = SJrNYrtiKK36454484;     SJrNYrtiKK36454484 = SJrNYrtiKK97043472;     SJrNYrtiKK97043472 = SJrNYrtiKK47055644;     SJrNYrtiKK47055644 = SJrNYrtiKK16967897;     SJrNYrtiKK16967897 = SJrNYrtiKK25636028;     SJrNYrtiKK25636028 = SJrNYrtiKK85237842;     SJrNYrtiKK85237842 = SJrNYrtiKK35466413;     SJrNYrtiKK35466413 = SJrNYrtiKK7009032;     SJrNYrtiKK7009032 = SJrNYrtiKK34679773;     SJrNYrtiKK34679773 = SJrNYrtiKK9312908;     SJrNYrtiKK9312908 = SJrNYrtiKK80927059;     SJrNYrtiKK80927059 = SJrNYrtiKK40793114;     SJrNYrtiKK40793114 = SJrNYrtiKK96931150;     SJrNYrtiKK96931150 = SJrNYrtiKK27761327;     SJrNYrtiKK27761327 = SJrNYrtiKK57430343;     SJrNYrtiKK57430343 = SJrNYrtiKK41933020;     SJrNYrtiKK41933020 = SJrNYrtiKK50133897;     SJrNYrtiKK50133897 = SJrNYrtiKK92670789;     SJrNYrtiKK92670789 = SJrNYrtiKK2801927;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void acCVUYwviS39609824() {     double pZMaHwtGeP42923382 = 81623632;    double pZMaHwtGeP1401663 = -557478252;    double pZMaHwtGeP8633513 = -413481147;    double pZMaHwtGeP30468624 = -442369713;    double pZMaHwtGeP76425353 = -593274210;    double pZMaHwtGeP17317890 = -195123950;    double pZMaHwtGeP21330100 = 96869027;    double pZMaHwtGeP61123429 = -473347178;    double pZMaHwtGeP72732939 = -361284154;    double pZMaHwtGeP79529773 = -183468027;    double pZMaHwtGeP26444017 = -592884570;    double pZMaHwtGeP98905712 = -652189386;    double pZMaHwtGeP7468013 = 42559046;    double pZMaHwtGeP24617423 = -897574190;    double pZMaHwtGeP29176922 = -189302342;    double pZMaHwtGeP59288791 = -101096665;    double pZMaHwtGeP93137084 = -17438010;    double pZMaHwtGeP97185225 = -972095779;    double pZMaHwtGeP98840503 = -559482339;    double pZMaHwtGeP61668404 = -118162748;    double pZMaHwtGeP29320004 = -637399715;    double pZMaHwtGeP99757038 = -963039300;    double pZMaHwtGeP8882083 = -302728567;    double pZMaHwtGeP36746239 = -17279384;    double pZMaHwtGeP60274053 = -462015610;    double pZMaHwtGeP11619034 = -317621907;    double pZMaHwtGeP12156980 = -497521755;    double pZMaHwtGeP62741226 = -959444888;    double pZMaHwtGeP35139849 = -257526570;    double pZMaHwtGeP45083803 = -108165406;    double pZMaHwtGeP58555492 = -243388567;    double pZMaHwtGeP60973236 = -557758236;    double pZMaHwtGeP97844213 = -627176286;    double pZMaHwtGeP27184760 = -87628121;    double pZMaHwtGeP89222764 = -311909167;    double pZMaHwtGeP86394415 = -440349896;    double pZMaHwtGeP84570846 = -228652175;    double pZMaHwtGeP8825908 = -795714667;    double pZMaHwtGeP79152358 = -48810634;    double pZMaHwtGeP18251915 = -841253216;    double pZMaHwtGeP19590703 = -547989186;    double pZMaHwtGeP93260363 = -69074521;    double pZMaHwtGeP93601188 = -564000058;    double pZMaHwtGeP47981650 = -530687635;    double pZMaHwtGeP59637061 = -900494804;    double pZMaHwtGeP99390920 = -315187431;    double pZMaHwtGeP20512502 = -803039749;    double pZMaHwtGeP88714554 = -677481107;    double pZMaHwtGeP93476169 = -552891233;    double pZMaHwtGeP11693017 = -654902291;    double pZMaHwtGeP57481727 = -211541871;    double pZMaHwtGeP17748234 = -287859605;    double pZMaHwtGeP28466118 = -715045080;    double pZMaHwtGeP142779 = -484756201;    double pZMaHwtGeP10289821 = -326771203;    double pZMaHwtGeP43166343 = 44662932;    double pZMaHwtGeP92519580 = -154749685;    double pZMaHwtGeP71887273 = -296201763;    double pZMaHwtGeP70194570 = -980354104;    double pZMaHwtGeP64806319 = -175652304;    double pZMaHwtGeP5160911 = -697602195;    double pZMaHwtGeP58588874 = 56313914;    double pZMaHwtGeP25983580 = -115820608;    double pZMaHwtGeP27649136 = -153118749;    double pZMaHwtGeP20974282 = -940079461;    double pZMaHwtGeP65470781 = 64873665;    double pZMaHwtGeP1061500 = 74986900;    double pZMaHwtGeP80283252 = -869812833;    double pZMaHwtGeP35394659 = -485665023;    double pZMaHwtGeP42782507 = -748952446;    double pZMaHwtGeP74717944 = -872444491;    double pZMaHwtGeP84311177 = -221723343;    double pZMaHwtGeP18032868 = -823285146;    double pZMaHwtGeP80588588 = -718229123;    double pZMaHwtGeP42077701 = -570173562;    double pZMaHwtGeP36059641 = -468325195;    double pZMaHwtGeP6155850 = -299039242;    double pZMaHwtGeP60900433 = -772040933;    double pZMaHwtGeP77109178 = -116784581;    double pZMaHwtGeP60883133 = -46828179;    double pZMaHwtGeP91106532 = -514582158;    double pZMaHwtGeP23442426 = -820040649;    double pZMaHwtGeP69265056 = -306553656;    double pZMaHwtGeP23446833 = -602624280;    double pZMaHwtGeP87602075 = -896623536;    double pZMaHwtGeP40807258 = -955528963;    double pZMaHwtGeP32507119 = -842713156;    double pZMaHwtGeP97701435 = -42420085;    double pZMaHwtGeP16894940 = -760856919;    double pZMaHwtGeP46056421 = -256572099;    double pZMaHwtGeP93874835 = -185600211;    double pZMaHwtGeP12683573 = -932450412;    double pZMaHwtGeP38631337 = -815360564;    double pZMaHwtGeP14346039 = -873158330;    double pZMaHwtGeP13091004 = -43651022;    double pZMaHwtGeP61001829 = -504303101;    double pZMaHwtGeP67276783 = -953253913;    double pZMaHwtGeP65952053 = -310881310;    double pZMaHwtGeP27007369 = -590608175;    double pZMaHwtGeP94166280 = 81623632;     pZMaHwtGeP42923382 = pZMaHwtGeP1401663;     pZMaHwtGeP1401663 = pZMaHwtGeP8633513;     pZMaHwtGeP8633513 = pZMaHwtGeP30468624;     pZMaHwtGeP30468624 = pZMaHwtGeP76425353;     pZMaHwtGeP76425353 = pZMaHwtGeP17317890;     pZMaHwtGeP17317890 = pZMaHwtGeP21330100;     pZMaHwtGeP21330100 = pZMaHwtGeP61123429;     pZMaHwtGeP61123429 = pZMaHwtGeP72732939;     pZMaHwtGeP72732939 = pZMaHwtGeP79529773;     pZMaHwtGeP79529773 = pZMaHwtGeP26444017;     pZMaHwtGeP26444017 = pZMaHwtGeP98905712;     pZMaHwtGeP98905712 = pZMaHwtGeP7468013;     pZMaHwtGeP7468013 = pZMaHwtGeP24617423;     pZMaHwtGeP24617423 = pZMaHwtGeP29176922;     pZMaHwtGeP29176922 = pZMaHwtGeP59288791;     pZMaHwtGeP59288791 = pZMaHwtGeP93137084;     pZMaHwtGeP93137084 = pZMaHwtGeP97185225;     pZMaHwtGeP97185225 = pZMaHwtGeP98840503;     pZMaHwtGeP98840503 = pZMaHwtGeP61668404;     pZMaHwtGeP61668404 = pZMaHwtGeP29320004;     pZMaHwtGeP29320004 = pZMaHwtGeP99757038;     pZMaHwtGeP99757038 = pZMaHwtGeP8882083;     pZMaHwtGeP8882083 = pZMaHwtGeP36746239;     pZMaHwtGeP36746239 = pZMaHwtGeP60274053;     pZMaHwtGeP60274053 = pZMaHwtGeP11619034;     pZMaHwtGeP11619034 = pZMaHwtGeP12156980;     pZMaHwtGeP12156980 = pZMaHwtGeP62741226;     pZMaHwtGeP62741226 = pZMaHwtGeP35139849;     pZMaHwtGeP35139849 = pZMaHwtGeP45083803;     pZMaHwtGeP45083803 = pZMaHwtGeP58555492;     pZMaHwtGeP58555492 = pZMaHwtGeP60973236;     pZMaHwtGeP60973236 = pZMaHwtGeP97844213;     pZMaHwtGeP97844213 = pZMaHwtGeP27184760;     pZMaHwtGeP27184760 = pZMaHwtGeP89222764;     pZMaHwtGeP89222764 = pZMaHwtGeP86394415;     pZMaHwtGeP86394415 = pZMaHwtGeP84570846;     pZMaHwtGeP84570846 = pZMaHwtGeP8825908;     pZMaHwtGeP8825908 = pZMaHwtGeP79152358;     pZMaHwtGeP79152358 = pZMaHwtGeP18251915;     pZMaHwtGeP18251915 = pZMaHwtGeP19590703;     pZMaHwtGeP19590703 = pZMaHwtGeP93260363;     pZMaHwtGeP93260363 = pZMaHwtGeP93601188;     pZMaHwtGeP93601188 = pZMaHwtGeP47981650;     pZMaHwtGeP47981650 = pZMaHwtGeP59637061;     pZMaHwtGeP59637061 = pZMaHwtGeP99390920;     pZMaHwtGeP99390920 = pZMaHwtGeP20512502;     pZMaHwtGeP20512502 = pZMaHwtGeP88714554;     pZMaHwtGeP88714554 = pZMaHwtGeP93476169;     pZMaHwtGeP93476169 = pZMaHwtGeP11693017;     pZMaHwtGeP11693017 = pZMaHwtGeP57481727;     pZMaHwtGeP57481727 = pZMaHwtGeP17748234;     pZMaHwtGeP17748234 = pZMaHwtGeP28466118;     pZMaHwtGeP28466118 = pZMaHwtGeP142779;     pZMaHwtGeP142779 = pZMaHwtGeP10289821;     pZMaHwtGeP10289821 = pZMaHwtGeP43166343;     pZMaHwtGeP43166343 = pZMaHwtGeP92519580;     pZMaHwtGeP92519580 = pZMaHwtGeP71887273;     pZMaHwtGeP71887273 = pZMaHwtGeP70194570;     pZMaHwtGeP70194570 = pZMaHwtGeP64806319;     pZMaHwtGeP64806319 = pZMaHwtGeP5160911;     pZMaHwtGeP5160911 = pZMaHwtGeP58588874;     pZMaHwtGeP58588874 = pZMaHwtGeP25983580;     pZMaHwtGeP25983580 = pZMaHwtGeP27649136;     pZMaHwtGeP27649136 = pZMaHwtGeP20974282;     pZMaHwtGeP20974282 = pZMaHwtGeP65470781;     pZMaHwtGeP65470781 = pZMaHwtGeP1061500;     pZMaHwtGeP1061500 = pZMaHwtGeP80283252;     pZMaHwtGeP80283252 = pZMaHwtGeP35394659;     pZMaHwtGeP35394659 = pZMaHwtGeP42782507;     pZMaHwtGeP42782507 = pZMaHwtGeP74717944;     pZMaHwtGeP74717944 = pZMaHwtGeP84311177;     pZMaHwtGeP84311177 = pZMaHwtGeP18032868;     pZMaHwtGeP18032868 = pZMaHwtGeP80588588;     pZMaHwtGeP80588588 = pZMaHwtGeP42077701;     pZMaHwtGeP42077701 = pZMaHwtGeP36059641;     pZMaHwtGeP36059641 = pZMaHwtGeP6155850;     pZMaHwtGeP6155850 = pZMaHwtGeP60900433;     pZMaHwtGeP60900433 = pZMaHwtGeP77109178;     pZMaHwtGeP77109178 = pZMaHwtGeP60883133;     pZMaHwtGeP60883133 = pZMaHwtGeP91106532;     pZMaHwtGeP91106532 = pZMaHwtGeP23442426;     pZMaHwtGeP23442426 = pZMaHwtGeP69265056;     pZMaHwtGeP69265056 = pZMaHwtGeP23446833;     pZMaHwtGeP23446833 = pZMaHwtGeP87602075;     pZMaHwtGeP87602075 = pZMaHwtGeP40807258;     pZMaHwtGeP40807258 = pZMaHwtGeP32507119;     pZMaHwtGeP32507119 = pZMaHwtGeP97701435;     pZMaHwtGeP97701435 = pZMaHwtGeP16894940;     pZMaHwtGeP16894940 = pZMaHwtGeP46056421;     pZMaHwtGeP46056421 = pZMaHwtGeP93874835;     pZMaHwtGeP93874835 = pZMaHwtGeP12683573;     pZMaHwtGeP12683573 = pZMaHwtGeP38631337;     pZMaHwtGeP38631337 = pZMaHwtGeP14346039;     pZMaHwtGeP14346039 = pZMaHwtGeP13091004;     pZMaHwtGeP13091004 = pZMaHwtGeP61001829;     pZMaHwtGeP61001829 = pZMaHwtGeP67276783;     pZMaHwtGeP67276783 = pZMaHwtGeP65952053;     pZMaHwtGeP65952053 = pZMaHwtGeP27007369;     pZMaHwtGeP27007369 = pZMaHwtGeP94166280;     pZMaHwtGeP94166280 = pZMaHwtGeP42923382;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void bLAXlGBLpb49425014() {     double nDRpQlhFCi92642551 = -692151014;    double nDRpQlhFCi55318736 = -763857363;    double nDRpQlhFCi91784205 = -52980838;    double nDRpQlhFCi92382887 = -861422831;    double nDRpQlhFCi5195367 = -669282332;    double nDRpQlhFCi13184132 = -895733137;    double nDRpQlhFCi97625688 = -372662265;    double nDRpQlhFCi99434301 = -316094935;    double nDRpQlhFCi51728152 = -208672325;    double nDRpQlhFCi65129715 = -63004842;    double nDRpQlhFCi38051070 = -382583202;    double nDRpQlhFCi75472260 = -94537942;    double nDRpQlhFCi32495165 = -736684570;    double nDRpQlhFCi98332225 = -972469947;    double nDRpQlhFCi47145154 = -348306213;    double nDRpQlhFCi67505060 = -323247252;    double nDRpQlhFCi986749 = -116806817;    double nDRpQlhFCi44709535 = -332816983;    double nDRpQlhFCi411232 = -374003539;    double nDRpQlhFCi44946643 = -634461936;    double nDRpQlhFCi37538237 = -30078814;    double nDRpQlhFCi55476352 = -893533754;    double nDRpQlhFCi52751524 = -152962488;    double nDRpQlhFCi31222630 = 29897742;    double nDRpQlhFCi47358591 = -225011058;    double nDRpQlhFCi83111944 = -282216081;    double nDRpQlhFCi32575176 = -856930603;    double nDRpQlhFCi68724532 = -845200024;    double nDRpQlhFCi52472413 = -582353406;    double nDRpQlhFCi19391285 = -967266726;    double nDRpQlhFCi3639553 = -308821056;    double nDRpQlhFCi95467339 = -571711536;    double nDRpQlhFCi98621504 = -458320429;    double nDRpQlhFCi33689266 = -69304077;    double nDRpQlhFCi70935678 = -840111779;    double nDRpQlhFCi39629413 = -544918780;    double nDRpQlhFCi31406776 = -637073403;    double nDRpQlhFCi66502368 = -603857637;    double nDRpQlhFCi26360117 = -381319917;    double nDRpQlhFCi21526158 = -274557781;    double nDRpQlhFCi85436023 = -216484008;    double nDRpQlhFCi10832838 = 14594786;    double nDRpQlhFCi74438952 = 47564388;    double nDRpQlhFCi32499045 = -597167784;    double nDRpQlhFCi94598820 = -145952530;    double nDRpQlhFCi4837027 = -397777849;    double nDRpQlhFCi20273412 = -484018227;    double nDRpQlhFCi13489198 = -791271267;    double nDRpQlhFCi20113666 = -504650154;    double nDRpQlhFCi66163526 = -126663527;    double nDRpQlhFCi43963186 = -161309643;    double nDRpQlhFCi75380122 = -964602571;    double nDRpQlhFCi41058262 = -62873529;    double nDRpQlhFCi31449499 = -584440364;    double nDRpQlhFCi11886778 = -297755206;    double nDRpQlhFCi37166200 = -798617261;    double nDRpQlhFCi2567212 = -510894876;    double nDRpQlhFCi60561576 = 17121420;    double nDRpQlhFCi45024297 = -536411773;    double nDRpQlhFCi22083423 = -287066252;    double nDRpQlhFCi80608955 = 61197465;    double nDRpQlhFCi28901156 = -527462242;    double nDRpQlhFCi46961888 = -733741530;    double nDRpQlhFCi32336868 = -241405599;    double nDRpQlhFCi61490163 = -754183786;    double nDRpQlhFCi42583731 = -810871666;    double nDRpQlhFCi76850755 = -636217514;    double nDRpQlhFCi98805898 = -567380494;    double nDRpQlhFCi27396547 = -32358168;    double nDRpQlhFCi7515742 = -803387434;    double nDRpQlhFCi36098284 = -686173850;    double nDRpQlhFCi34484381 = -512949180;    double nDRpQlhFCi18349419 = -951497067;    double nDRpQlhFCi78885074 = 554241;    double nDRpQlhFCi59510619 = -317977929;    double nDRpQlhFCi26705399 = 55326399;    double nDRpQlhFCi81037399 = -841098143;    double nDRpQlhFCi20252480 = -555794704;    double nDRpQlhFCi36623810 = -824149728;    double nDRpQlhFCi42521564 = -827233209;    double nDRpQlhFCi62838532 = -798197854;    double nDRpQlhFCi19085979 = 34340664;    double nDRpQlhFCi48610867 = -240549871;    double nDRpQlhFCi86308886 = -355689879;    double nDRpQlhFCi75428098 = -705957084;    double nDRpQlhFCi28259430 = -344218486;    double nDRpQlhFCi54409077 = -408838007;    double nDRpQlhFCi67172006 = -873880065;    double nDRpQlhFCi21802489 = -771548872;    double nDRpQlhFCi33769479 = 58505481;    double nDRpQlhFCi37062201 = 65976096;    double nDRpQlhFCi70845200 = -554194823;    double nDRpQlhFCi21478072 = 32554136;    double nDRpQlhFCi4276694 = 5746335;    double nDRpQlhFCi40917202 = -235755246;    double nDRpQlhFCi56534867 = -689021766;    double nDRpQlhFCi63870950 = -251663685;    double nDRpQlhFCi42102085 = -711030013;    double nDRpQlhFCi71008882 = -842983999;    double nDRpQlhFCi52015089 = -692151014;     nDRpQlhFCi92642551 = nDRpQlhFCi55318736;     nDRpQlhFCi55318736 = nDRpQlhFCi91784205;     nDRpQlhFCi91784205 = nDRpQlhFCi92382887;     nDRpQlhFCi92382887 = nDRpQlhFCi5195367;     nDRpQlhFCi5195367 = nDRpQlhFCi13184132;     nDRpQlhFCi13184132 = nDRpQlhFCi97625688;     nDRpQlhFCi97625688 = nDRpQlhFCi99434301;     nDRpQlhFCi99434301 = nDRpQlhFCi51728152;     nDRpQlhFCi51728152 = nDRpQlhFCi65129715;     nDRpQlhFCi65129715 = nDRpQlhFCi38051070;     nDRpQlhFCi38051070 = nDRpQlhFCi75472260;     nDRpQlhFCi75472260 = nDRpQlhFCi32495165;     nDRpQlhFCi32495165 = nDRpQlhFCi98332225;     nDRpQlhFCi98332225 = nDRpQlhFCi47145154;     nDRpQlhFCi47145154 = nDRpQlhFCi67505060;     nDRpQlhFCi67505060 = nDRpQlhFCi986749;     nDRpQlhFCi986749 = nDRpQlhFCi44709535;     nDRpQlhFCi44709535 = nDRpQlhFCi411232;     nDRpQlhFCi411232 = nDRpQlhFCi44946643;     nDRpQlhFCi44946643 = nDRpQlhFCi37538237;     nDRpQlhFCi37538237 = nDRpQlhFCi55476352;     nDRpQlhFCi55476352 = nDRpQlhFCi52751524;     nDRpQlhFCi52751524 = nDRpQlhFCi31222630;     nDRpQlhFCi31222630 = nDRpQlhFCi47358591;     nDRpQlhFCi47358591 = nDRpQlhFCi83111944;     nDRpQlhFCi83111944 = nDRpQlhFCi32575176;     nDRpQlhFCi32575176 = nDRpQlhFCi68724532;     nDRpQlhFCi68724532 = nDRpQlhFCi52472413;     nDRpQlhFCi52472413 = nDRpQlhFCi19391285;     nDRpQlhFCi19391285 = nDRpQlhFCi3639553;     nDRpQlhFCi3639553 = nDRpQlhFCi95467339;     nDRpQlhFCi95467339 = nDRpQlhFCi98621504;     nDRpQlhFCi98621504 = nDRpQlhFCi33689266;     nDRpQlhFCi33689266 = nDRpQlhFCi70935678;     nDRpQlhFCi70935678 = nDRpQlhFCi39629413;     nDRpQlhFCi39629413 = nDRpQlhFCi31406776;     nDRpQlhFCi31406776 = nDRpQlhFCi66502368;     nDRpQlhFCi66502368 = nDRpQlhFCi26360117;     nDRpQlhFCi26360117 = nDRpQlhFCi21526158;     nDRpQlhFCi21526158 = nDRpQlhFCi85436023;     nDRpQlhFCi85436023 = nDRpQlhFCi10832838;     nDRpQlhFCi10832838 = nDRpQlhFCi74438952;     nDRpQlhFCi74438952 = nDRpQlhFCi32499045;     nDRpQlhFCi32499045 = nDRpQlhFCi94598820;     nDRpQlhFCi94598820 = nDRpQlhFCi4837027;     nDRpQlhFCi4837027 = nDRpQlhFCi20273412;     nDRpQlhFCi20273412 = nDRpQlhFCi13489198;     nDRpQlhFCi13489198 = nDRpQlhFCi20113666;     nDRpQlhFCi20113666 = nDRpQlhFCi66163526;     nDRpQlhFCi66163526 = nDRpQlhFCi43963186;     nDRpQlhFCi43963186 = nDRpQlhFCi75380122;     nDRpQlhFCi75380122 = nDRpQlhFCi41058262;     nDRpQlhFCi41058262 = nDRpQlhFCi31449499;     nDRpQlhFCi31449499 = nDRpQlhFCi11886778;     nDRpQlhFCi11886778 = nDRpQlhFCi37166200;     nDRpQlhFCi37166200 = nDRpQlhFCi2567212;     nDRpQlhFCi2567212 = nDRpQlhFCi60561576;     nDRpQlhFCi60561576 = nDRpQlhFCi45024297;     nDRpQlhFCi45024297 = nDRpQlhFCi22083423;     nDRpQlhFCi22083423 = nDRpQlhFCi80608955;     nDRpQlhFCi80608955 = nDRpQlhFCi28901156;     nDRpQlhFCi28901156 = nDRpQlhFCi46961888;     nDRpQlhFCi46961888 = nDRpQlhFCi32336868;     nDRpQlhFCi32336868 = nDRpQlhFCi61490163;     nDRpQlhFCi61490163 = nDRpQlhFCi42583731;     nDRpQlhFCi42583731 = nDRpQlhFCi76850755;     nDRpQlhFCi76850755 = nDRpQlhFCi98805898;     nDRpQlhFCi98805898 = nDRpQlhFCi27396547;     nDRpQlhFCi27396547 = nDRpQlhFCi7515742;     nDRpQlhFCi7515742 = nDRpQlhFCi36098284;     nDRpQlhFCi36098284 = nDRpQlhFCi34484381;     nDRpQlhFCi34484381 = nDRpQlhFCi18349419;     nDRpQlhFCi18349419 = nDRpQlhFCi78885074;     nDRpQlhFCi78885074 = nDRpQlhFCi59510619;     nDRpQlhFCi59510619 = nDRpQlhFCi26705399;     nDRpQlhFCi26705399 = nDRpQlhFCi81037399;     nDRpQlhFCi81037399 = nDRpQlhFCi20252480;     nDRpQlhFCi20252480 = nDRpQlhFCi36623810;     nDRpQlhFCi36623810 = nDRpQlhFCi42521564;     nDRpQlhFCi42521564 = nDRpQlhFCi62838532;     nDRpQlhFCi62838532 = nDRpQlhFCi19085979;     nDRpQlhFCi19085979 = nDRpQlhFCi48610867;     nDRpQlhFCi48610867 = nDRpQlhFCi86308886;     nDRpQlhFCi86308886 = nDRpQlhFCi75428098;     nDRpQlhFCi75428098 = nDRpQlhFCi28259430;     nDRpQlhFCi28259430 = nDRpQlhFCi54409077;     nDRpQlhFCi54409077 = nDRpQlhFCi67172006;     nDRpQlhFCi67172006 = nDRpQlhFCi21802489;     nDRpQlhFCi21802489 = nDRpQlhFCi33769479;     nDRpQlhFCi33769479 = nDRpQlhFCi37062201;     nDRpQlhFCi37062201 = nDRpQlhFCi70845200;     nDRpQlhFCi70845200 = nDRpQlhFCi21478072;     nDRpQlhFCi21478072 = nDRpQlhFCi4276694;     nDRpQlhFCi4276694 = nDRpQlhFCi40917202;     nDRpQlhFCi40917202 = nDRpQlhFCi56534867;     nDRpQlhFCi56534867 = nDRpQlhFCi63870950;     nDRpQlhFCi63870950 = nDRpQlhFCi42102085;     nDRpQlhFCi42102085 = nDRpQlhFCi71008882;     nDRpQlhFCi71008882 = nDRpQlhFCi52015089;     nDRpQlhFCi52015089 = nDRpQlhFCi92642551;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void VWFQxwnHxA56083759() {     double BTwWTcufES92636080 = -564675158;    double BTwWTcufES93891052 = -806452495;    double BTwWTcufES93097417 = -10109451;    double BTwWTcufES77462748 = -681963658;    double BTwWTcufES14035469 = -37637662;    double BTwWTcufES47989564 = -595165949;    double BTwWTcufES66173024 = -77198732;    double BTwWTcufES68648352 = -20272268;    double BTwWTcufES34468317 = 77681604;    double BTwWTcufES61769229 = -799811713;    double BTwWTcufES58371675 = -342055275;    double BTwWTcufES89763937 = -930016314;    double BTwWTcufES64348078 = -758343044;    double BTwWTcufES52176445 = -217622999;    double BTwWTcufES92317262 = -311019084;    double BTwWTcufES42069608 = -656042903;    double BTwWTcufES56839508 = 63578614;    double BTwWTcufES67710068 = -729888609;    double BTwWTcufES20839502 = -991985805;    double BTwWTcufES17020406 = -146486388;    double BTwWTcufES17207662 = -840545193;    double BTwWTcufES33772618 = -303525638;    double BTwWTcufES89629921 = -611920330;    double BTwWTcufES74872738 = -951022780;    double BTwWTcufES75801281 = -651704923;    double BTwWTcufES99686021 = -190422005;    double BTwWTcufES2796758 = -57730316;    double BTwWTcufES4128058 = -354167018;    double BTwWTcufES65150860 = -128423706;    double BTwWTcufES95481154 = -642698100;    double BTwWTcufES37052231 = -695914291;    double BTwWTcufES80191197 = -963533295;    double BTwWTcufES84073928 = -660264642;    double BTwWTcufES33972103 = 54014191;    double BTwWTcufES53293717 = 574299;    double BTwWTcufES82457602 = -624684507;    double BTwWTcufES43348144 = -736726781;    double BTwWTcufES36420048 = -599120319;    double BTwWTcufES61673713 = 5998679;    double BTwWTcufES93399049 = -983989467;    double BTwWTcufES73291691 = -304427906;    double BTwWTcufES96530498 = -179197276;    double BTwWTcufES34722312 = -511565864;    double BTwWTcufES91730943 = -154132228;    double BTwWTcufES78616872 = -619211930;    double BTwWTcufES45264995 = -209822501;    double BTwWTcufES47971203 = -965298849;    double BTwWTcufES88872778 = -300032623;    double BTwWTcufES38772055 = -68413991;    double BTwWTcufES86208949 = -813688419;    double BTwWTcufES57222556 = -839864650;    double BTwWTcufES89927379 = -246748315;    double BTwWTcufES16506610 = 45072459;    double BTwWTcufES41699770 = -16360215;    double BTwWTcufES38876098 = -54654478;    double BTwWTcufES58863462 = -161149521;    double BTwWTcufES4261132 = -94532165;    double BTwWTcufES18224680 = -59086671;    double BTwWTcufES1661468 = 69741265;    double BTwWTcufES14349447 = -847215657;    double BTwWTcufES45192806 = -437435634;    double BTwWTcufES62044967 = -723031714;    double BTwWTcufES3497492 = -891848563;    double BTwWTcufES38987162 = -279620296;    double BTwWTcufES24716998 = -3897423;    double BTwWTcufES78180478 = -378521980;    double BTwWTcufES5690010 = -169751673;    double BTwWTcufES30375976 = -712357236;    double BTwWTcufES98882728 = -118197299;    double BTwWTcufES9859661 = -686334577;    double BTwWTcufES98721463 = -919316122;    double BTwWTcufES20419461 = -337301068;    double BTwWTcufES6036356 = -635887289;    double BTwWTcufES27440452 = 92003661;    double BTwWTcufES43728715 = -842058482;    double BTwWTcufES20677163 = -561347918;    double BTwWTcufES99050305 = -791959774;    double BTwWTcufES97898977 = -357788103;    double BTwWTcufES96255865 = -231810850;    double BTwWTcufES30536286 = -341882423;    double BTwWTcufES51714818 = -225123156;    double BTwWTcufES13923979 = -757697694;    double BTwWTcufES65356002 = -185753028;    double BTwWTcufES78941911 = -314735287;    double BTwWTcufES38258599 = -802833450;    double BTwWTcufES47124851 = -349165976;    double BTwWTcufES63684587 = -908605755;    double BTwWTcufES42374158 = -543904427;    double BTwWTcufES95096005 = -891331332;    double BTwWTcufES94430254 = -838276180;    double BTwWTcufES78196471 = -430152343;    double BTwWTcufES25123465 = -577640110;    double BTwWTcufES34758580 = -568861585;    double BTwWTcufES47324266 = -146785664;    double BTwWTcufES48206244 = -446553834;    double BTwWTcufES11246724 = -581396192;    double BTwWTcufES93033007 = -287348714;    double BTwWTcufES95735150 = -131945568;    double BTwWTcufES67013945 = -50234805;    double BTwWTcufES436394 = -564675158;     BTwWTcufES92636080 = BTwWTcufES93891052;     BTwWTcufES93891052 = BTwWTcufES93097417;     BTwWTcufES93097417 = BTwWTcufES77462748;     BTwWTcufES77462748 = BTwWTcufES14035469;     BTwWTcufES14035469 = BTwWTcufES47989564;     BTwWTcufES47989564 = BTwWTcufES66173024;     BTwWTcufES66173024 = BTwWTcufES68648352;     BTwWTcufES68648352 = BTwWTcufES34468317;     BTwWTcufES34468317 = BTwWTcufES61769229;     BTwWTcufES61769229 = BTwWTcufES58371675;     BTwWTcufES58371675 = BTwWTcufES89763937;     BTwWTcufES89763937 = BTwWTcufES64348078;     BTwWTcufES64348078 = BTwWTcufES52176445;     BTwWTcufES52176445 = BTwWTcufES92317262;     BTwWTcufES92317262 = BTwWTcufES42069608;     BTwWTcufES42069608 = BTwWTcufES56839508;     BTwWTcufES56839508 = BTwWTcufES67710068;     BTwWTcufES67710068 = BTwWTcufES20839502;     BTwWTcufES20839502 = BTwWTcufES17020406;     BTwWTcufES17020406 = BTwWTcufES17207662;     BTwWTcufES17207662 = BTwWTcufES33772618;     BTwWTcufES33772618 = BTwWTcufES89629921;     BTwWTcufES89629921 = BTwWTcufES74872738;     BTwWTcufES74872738 = BTwWTcufES75801281;     BTwWTcufES75801281 = BTwWTcufES99686021;     BTwWTcufES99686021 = BTwWTcufES2796758;     BTwWTcufES2796758 = BTwWTcufES4128058;     BTwWTcufES4128058 = BTwWTcufES65150860;     BTwWTcufES65150860 = BTwWTcufES95481154;     BTwWTcufES95481154 = BTwWTcufES37052231;     BTwWTcufES37052231 = BTwWTcufES80191197;     BTwWTcufES80191197 = BTwWTcufES84073928;     BTwWTcufES84073928 = BTwWTcufES33972103;     BTwWTcufES33972103 = BTwWTcufES53293717;     BTwWTcufES53293717 = BTwWTcufES82457602;     BTwWTcufES82457602 = BTwWTcufES43348144;     BTwWTcufES43348144 = BTwWTcufES36420048;     BTwWTcufES36420048 = BTwWTcufES61673713;     BTwWTcufES61673713 = BTwWTcufES93399049;     BTwWTcufES93399049 = BTwWTcufES73291691;     BTwWTcufES73291691 = BTwWTcufES96530498;     BTwWTcufES96530498 = BTwWTcufES34722312;     BTwWTcufES34722312 = BTwWTcufES91730943;     BTwWTcufES91730943 = BTwWTcufES78616872;     BTwWTcufES78616872 = BTwWTcufES45264995;     BTwWTcufES45264995 = BTwWTcufES47971203;     BTwWTcufES47971203 = BTwWTcufES88872778;     BTwWTcufES88872778 = BTwWTcufES38772055;     BTwWTcufES38772055 = BTwWTcufES86208949;     BTwWTcufES86208949 = BTwWTcufES57222556;     BTwWTcufES57222556 = BTwWTcufES89927379;     BTwWTcufES89927379 = BTwWTcufES16506610;     BTwWTcufES16506610 = BTwWTcufES41699770;     BTwWTcufES41699770 = BTwWTcufES38876098;     BTwWTcufES38876098 = BTwWTcufES58863462;     BTwWTcufES58863462 = BTwWTcufES4261132;     BTwWTcufES4261132 = BTwWTcufES18224680;     BTwWTcufES18224680 = BTwWTcufES1661468;     BTwWTcufES1661468 = BTwWTcufES14349447;     BTwWTcufES14349447 = BTwWTcufES45192806;     BTwWTcufES45192806 = BTwWTcufES62044967;     BTwWTcufES62044967 = BTwWTcufES3497492;     BTwWTcufES3497492 = BTwWTcufES38987162;     BTwWTcufES38987162 = BTwWTcufES24716998;     BTwWTcufES24716998 = BTwWTcufES78180478;     BTwWTcufES78180478 = BTwWTcufES5690010;     BTwWTcufES5690010 = BTwWTcufES30375976;     BTwWTcufES30375976 = BTwWTcufES98882728;     BTwWTcufES98882728 = BTwWTcufES9859661;     BTwWTcufES9859661 = BTwWTcufES98721463;     BTwWTcufES98721463 = BTwWTcufES20419461;     BTwWTcufES20419461 = BTwWTcufES6036356;     BTwWTcufES6036356 = BTwWTcufES27440452;     BTwWTcufES27440452 = BTwWTcufES43728715;     BTwWTcufES43728715 = BTwWTcufES20677163;     BTwWTcufES20677163 = BTwWTcufES99050305;     BTwWTcufES99050305 = BTwWTcufES97898977;     BTwWTcufES97898977 = BTwWTcufES96255865;     BTwWTcufES96255865 = BTwWTcufES30536286;     BTwWTcufES30536286 = BTwWTcufES51714818;     BTwWTcufES51714818 = BTwWTcufES13923979;     BTwWTcufES13923979 = BTwWTcufES65356002;     BTwWTcufES65356002 = BTwWTcufES78941911;     BTwWTcufES78941911 = BTwWTcufES38258599;     BTwWTcufES38258599 = BTwWTcufES47124851;     BTwWTcufES47124851 = BTwWTcufES63684587;     BTwWTcufES63684587 = BTwWTcufES42374158;     BTwWTcufES42374158 = BTwWTcufES95096005;     BTwWTcufES95096005 = BTwWTcufES94430254;     BTwWTcufES94430254 = BTwWTcufES78196471;     BTwWTcufES78196471 = BTwWTcufES25123465;     BTwWTcufES25123465 = BTwWTcufES34758580;     BTwWTcufES34758580 = BTwWTcufES47324266;     BTwWTcufES47324266 = BTwWTcufES48206244;     BTwWTcufES48206244 = BTwWTcufES11246724;     BTwWTcufES11246724 = BTwWTcufES93033007;     BTwWTcufES93033007 = BTwWTcufES95735150;     BTwWTcufES95735150 = BTwWTcufES67013945;     BTwWTcufES67013945 = BTwWTcufES436394;     BTwWTcufES436394 = BTwWTcufES92636080;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void ZVnfAKIzEt10499972() {     double ZcjYMfwCaB63287747 = -82824288;    double ZcjYMfwCaB62564569 = -678095794;    double ZcjYMfwCaB73066336 = -452403911;    double ZcjYMfwCaB17394035 = -872102047;    double ZcjYMfwCaB35485790 = -921044009;    double ZcjYMfwCaB37989326 = -296747101;    double ZcjYMfwCaB84551920 = -75974442;    double ZcjYMfwCaB25490733 = -367089358;    double ZcjYMfwCaB80333370 = -15665725;    double ZcjYMfwCaB84735689 = -907064;    double ZcjYMfwCaB620837 = -376957204;    double ZcjYMfwCaB35310509 = -145780107;    double ZcjYMfwCaB44175466 = -667880103;    double ZcjYMfwCaB36023479 = -18651209;    double ZcjYMfwCaB75037282 = -460769087;    double ZcjYMfwCaB74598248 = -226026489;    double ZcjYMfwCaB50692335 = -945606516;    double ZcjYMfwCaB88003562 = -719054805;    double ZcjYMfwCaB2547284 = -599173747;    double ZcjYMfwCaB75807115 = -688852692;    double ZcjYMfwCaB72343415 = -749224333;    double ZcjYMfwCaB95168382 = -634810943;    double ZcjYMfwCaB91573729 = -79763475;    double ZcjYMfwCaB8722275 = -543423687;    double ZcjYMfwCaB39925669 = -327609611;    double ZcjYMfwCaB30283602 = -288706450;    double ZcjYMfwCaB11057544 = -452908091;    double ZcjYMfwCaB11910283 = -859290786;    double ZcjYMfwCaB97293347 = -422781923;    double ZcjYMfwCaB34653251 = -101093171;    double ZcjYMfwCaB44124227 = -444347976;    double ZcjYMfwCaB24702658 = -191830458;    double ZcjYMfwCaB67019289 = -607226250;    double ZcjYMfwCaB99337078 = -609515494;    double ZcjYMfwCaB31317953 = -946372480;    double ZcjYMfwCaB42450862 = -703911055;    double ZcjYMfwCaB87139630 = -925319989;    double ZcjYMfwCaB89212536 = -383233427;    double ZcjYMfwCaB40207002 = 50270043;    double ZcjYMfwCaB88502347 = -501597830;    double ZcjYMfwCaB33364975 = -688446785;    double ZcjYMfwCaB25013490 = 51413655;    double ZcjYMfwCaB83689272 = -498319916;    double ZcjYMfwCaB98239243 = -226107199;    double ZcjYMfwCaB74570931 = -927858325;    double ZcjYMfwCaB51670830 = 31840344;    double ZcjYMfwCaB74102406 = 18631351;    double ZcjYMfwCaB54486989 = -752162918;    double ZcjYMfwCaB44213507 = -54177960;    double ZcjYMfwCaB94636435 = 3302703;    double ZcjYMfwCaB25411881 = -386436771;    double ZcjYMfwCaB31798884 = -215968037;    double ZcjYMfwCaB71582267 = -982725583;    double ZcjYMfwCaB31927075 = -677241712;    double ZcjYMfwCaB38156556 = -165595393;    double ZcjYMfwCaB68119364 = -448013346;    double ZcjYMfwCaB70990840 = -498332319;    double ZcjYMfwCaB64344062 = -908980225;    double ZcjYMfwCaB77468366 = -444492436;    double ZcjYMfwCaB5202188 = -532337560;    double ZcjYMfwCaB26931783 = -843839010;    double ZcjYMfwCaB72641637 = -216683657;    double ZcjYMfwCaB28197385 = -944307435;    double ZcjYMfwCaB45680120 = -914572554;    double ZcjYMfwCaB40611462 = -556559088;    double ZcjYMfwCaB75918178 = -85126747;    double ZcjYMfwCaB68291220 = -538553857;    double ZcjYMfwCaB44838388 = 41635390;    double ZcjYMfwCaB4705526 = -72278730;    double ZcjYMfwCaB32586421 = -756858032;    double ZcjYMfwCaB87458618 = -300706501;    double ZcjYMfwCaB61479798 = -462373090;    double ZcjYMfwCaB47796561 = -669324848;    double ZcjYMfwCaB14044936 = 2424082;    double ZcjYMfwCaB42442140 = 99594092;    double ZcjYMfwCaB47329926 = -700637989;    double ZcjYMfwCaB11479111 = -36491027;    double ZcjYMfwCaB93334486 = -853656276;    double ZcjYMfwCaB34151344 = -615565362;    double ZcjYMfwCaB88254839 = -259449955;    double ZcjYMfwCaB56181196 = -207337802;    double ZcjYMfwCaB56570555 = -700745174;    double ZcjYMfwCaB67696775 = -705112827;    double ZcjYMfwCaB2656913 = -326084626;    double ZcjYMfwCaB9241370 = -714656400;    double ZcjYMfwCaB12325343 = -128379940;    double ZcjYMfwCaB53120391 = -209104875;    double ZcjYMfwCaB35092215 = -929984539;    double ZcjYMfwCaB61180522 = -343920102;    double ZcjYMfwCaB63198588 = -398359135;    double ZcjYMfwCaB71460021 = -105578736;    double ZcjYMfwCaB22795568 = 83660236;    double ZcjYMfwCaB11744171 = -938740991;    double ZcjYMfwCaB35004814 = -417392398;    double ZcjYMfwCaB61570565 = -657758820;    double ZcjYMfwCaB60723337 = -371763128;    double ZcjYMfwCaB96816105 = -4278910;    double ZcjYMfwCaB38009152 = -583747362;    double ZcjYMfwCaB57627781 = -669548112;    double ZcjYMfwCaB98652752 = -82824288;     ZcjYMfwCaB63287747 = ZcjYMfwCaB62564569;     ZcjYMfwCaB62564569 = ZcjYMfwCaB73066336;     ZcjYMfwCaB73066336 = ZcjYMfwCaB17394035;     ZcjYMfwCaB17394035 = ZcjYMfwCaB35485790;     ZcjYMfwCaB35485790 = ZcjYMfwCaB37989326;     ZcjYMfwCaB37989326 = ZcjYMfwCaB84551920;     ZcjYMfwCaB84551920 = ZcjYMfwCaB25490733;     ZcjYMfwCaB25490733 = ZcjYMfwCaB80333370;     ZcjYMfwCaB80333370 = ZcjYMfwCaB84735689;     ZcjYMfwCaB84735689 = ZcjYMfwCaB620837;     ZcjYMfwCaB620837 = ZcjYMfwCaB35310509;     ZcjYMfwCaB35310509 = ZcjYMfwCaB44175466;     ZcjYMfwCaB44175466 = ZcjYMfwCaB36023479;     ZcjYMfwCaB36023479 = ZcjYMfwCaB75037282;     ZcjYMfwCaB75037282 = ZcjYMfwCaB74598248;     ZcjYMfwCaB74598248 = ZcjYMfwCaB50692335;     ZcjYMfwCaB50692335 = ZcjYMfwCaB88003562;     ZcjYMfwCaB88003562 = ZcjYMfwCaB2547284;     ZcjYMfwCaB2547284 = ZcjYMfwCaB75807115;     ZcjYMfwCaB75807115 = ZcjYMfwCaB72343415;     ZcjYMfwCaB72343415 = ZcjYMfwCaB95168382;     ZcjYMfwCaB95168382 = ZcjYMfwCaB91573729;     ZcjYMfwCaB91573729 = ZcjYMfwCaB8722275;     ZcjYMfwCaB8722275 = ZcjYMfwCaB39925669;     ZcjYMfwCaB39925669 = ZcjYMfwCaB30283602;     ZcjYMfwCaB30283602 = ZcjYMfwCaB11057544;     ZcjYMfwCaB11057544 = ZcjYMfwCaB11910283;     ZcjYMfwCaB11910283 = ZcjYMfwCaB97293347;     ZcjYMfwCaB97293347 = ZcjYMfwCaB34653251;     ZcjYMfwCaB34653251 = ZcjYMfwCaB44124227;     ZcjYMfwCaB44124227 = ZcjYMfwCaB24702658;     ZcjYMfwCaB24702658 = ZcjYMfwCaB67019289;     ZcjYMfwCaB67019289 = ZcjYMfwCaB99337078;     ZcjYMfwCaB99337078 = ZcjYMfwCaB31317953;     ZcjYMfwCaB31317953 = ZcjYMfwCaB42450862;     ZcjYMfwCaB42450862 = ZcjYMfwCaB87139630;     ZcjYMfwCaB87139630 = ZcjYMfwCaB89212536;     ZcjYMfwCaB89212536 = ZcjYMfwCaB40207002;     ZcjYMfwCaB40207002 = ZcjYMfwCaB88502347;     ZcjYMfwCaB88502347 = ZcjYMfwCaB33364975;     ZcjYMfwCaB33364975 = ZcjYMfwCaB25013490;     ZcjYMfwCaB25013490 = ZcjYMfwCaB83689272;     ZcjYMfwCaB83689272 = ZcjYMfwCaB98239243;     ZcjYMfwCaB98239243 = ZcjYMfwCaB74570931;     ZcjYMfwCaB74570931 = ZcjYMfwCaB51670830;     ZcjYMfwCaB51670830 = ZcjYMfwCaB74102406;     ZcjYMfwCaB74102406 = ZcjYMfwCaB54486989;     ZcjYMfwCaB54486989 = ZcjYMfwCaB44213507;     ZcjYMfwCaB44213507 = ZcjYMfwCaB94636435;     ZcjYMfwCaB94636435 = ZcjYMfwCaB25411881;     ZcjYMfwCaB25411881 = ZcjYMfwCaB31798884;     ZcjYMfwCaB31798884 = ZcjYMfwCaB71582267;     ZcjYMfwCaB71582267 = ZcjYMfwCaB31927075;     ZcjYMfwCaB31927075 = ZcjYMfwCaB38156556;     ZcjYMfwCaB38156556 = ZcjYMfwCaB68119364;     ZcjYMfwCaB68119364 = ZcjYMfwCaB70990840;     ZcjYMfwCaB70990840 = ZcjYMfwCaB64344062;     ZcjYMfwCaB64344062 = ZcjYMfwCaB77468366;     ZcjYMfwCaB77468366 = ZcjYMfwCaB5202188;     ZcjYMfwCaB5202188 = ZcjYMfwCaB26931783;     ZcjYMfwCaB26931783 = ZcjYMfwCaB72641637;     ZcjYMfwCaB72641637 = ZcjYMfwCaB28197385;     ZcjYMfwCaB28197385 = ZcjYMfwCaB45680120;     ZcjYMfwCaB45680120 = ZcjYMfwCaB40611462;     ZcjYMfwCaB40611462 = ZcjYMfwCaB75918178;     ZcjYMfwCaB75918178 = ZcjYMfwCaB68291220;     ZcjYMfwCaB68291220 = ZcjYMfwCaB44838388;     ZcjYMfwCaB44838388 = ZcjYMfwCaB4705526;     ZcjYMfwCaB4705526 = ZcjYMfwCaB32586421;     ZcjYMfwCaB32586421 = ZcjYMfwCaB87458618;     ZcjYMfwCaB87458618 = ZcjYMfwCaB61479798;     ZcjYMfwCaB61479798 = ZcjYMfwCaB47796561;     ZcjYMfwCaB47796561 = ZcjYMfwCaB14044936;     ZcjYMfwCaB14044936 = ZcjYMfwCaB42442140;     ZcjYMfwCaB42442140 = ZcjYMfwCaB47329926;     ZcjYMfwCaB47329926 = ZcjYMfwCaB11479111;     ZcjYMfwCaB11479111 = ZcjYMfwCaB93334486;     ZcjYMfwCaB93334486 = ZcjYMfwCaB34151344;     ZcjYMfwCaB34151344 = ZcjYMfwCaB88254839;     ZcjYMfwCaB88254839 = ZcjYMfwCaB56181196;     ZcjYMfwCaB56181196 = ZcjYMfwCaB56570555;     ZcjYMfwCaB56570555 = ZcjYMfwCaB67696775;     ZcjYMfwCaB67696775 = ZcjYMfwCaB2656913;     ZcjYMfwCaB2656913 = ZcjYMfwCaB9241370;     ZcjYMfwCaB9241370 = ZcjYMfwCaB12325343;     ZcjYMfwCaB12325343 = ZcjYMfwCaB53120391;     ZcjYMfwCaB53120391 = ZcjYMfwCaB35092215;     ZcjYMfwCaB35092215 = ZcjYMfwCaB61180522;     ZcjYMfwCaB61180522 = ZcjYMfwCaB63198588;     ZcjYMfwCaB63198588 = ZcjYMfwCaB71460021;     ZcjYMfwCaB71460021 = ZcjYMfwCaB22795568;     ZcjYMfwCaB22795568 = ZcjYMfwCaB11744171;     ZcjYMfwCaB11744171 = ZcjYMfwCaB35004814;     ZcjYMfwCaB35004814 = ZcjYMfwCaB61570565;     ZcjYMfwCaB61570565 = ZcjYMfwCaB60723337;     ZcjYMfwCaB60723337 = ZcjYMfwCaB96816105;     ZcjYMfwCaB96816105 = ZcjYMfwCaB38009152;     ZcjYMfwCaB38009152 = ZcjYMfwCaB57627781;     ZcjYMfwCaB57627781 = ZcjYMfwCaB98652752;     ZcjYMfwCaB98652752 = ZcjYMfwCaB63287747;}
// Junk Finished
