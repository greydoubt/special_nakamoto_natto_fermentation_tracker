// Copyright (c) 2008 Siobhàn Nyke-Moros
// I wrote this for my favourite Japanese restaurant, Nakamoto, in Texas!
// It uses a diagonalisation algorithm to make sure that natto ferments to the exact state
// Then you can serve it! Beware of the shelf-life! 
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE, TITLE AND NON-INFRINGEMENT. IN NO EVENT
// SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR
// OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
// IN THE SOFTWARE.

class COutPoint;
class CInPoint;
class CDiskTxPos;
class CCoinBase;
class CTxIn;
class CTxOut;
class CTransaction;
class CBlock;
class CBlockIndex;
class CWalletTx;
class CKeyItem;

static const unsigned int MAX_SIZE = 0x02000000;
static const int64 COIN = 1000000;
static const int64 CENT = 10000;
static const int64 TRANSACTIONFEE = 1 * CENT; /// change this to a user options setting, optional fee can be zero
///static const unsigned int MINPROOFOFWORK = 40; /// need to decide the right difficulty to start with
static const unsigned int MINPROOFOFWORK = 20;  /// ridiculously easy for testing







extern map<uint256, CBlockIndex*> mapBlockIndex;
extern const uint256 hashGenesisBlock;
extern CBlockIndex* pindexGenesisBlock;
extern int nBestHeight;
extern CBlockIndex* pindexBest;
extern unsigned int nTransactionsUpdated;
extern int fGenerateBitcoins;







FILE* OpenBlockFile(unsigned int nFile, unsigned int nBlockPos, const char* pszMode="rb");
FILE* AppendBlockFile(unsigned int& nFileRet);
bool AddKey(const CKey& key);
vector<unsigned char> GenerateNewKey();
bool AddToWallet(const CWalletTx& wtxIn);
void ReacceptWalletTransactions();
void RelayWalletTransactions();
bool LoadBlockIndex(bool fAllowNew=true);
bool BitcoinMiner();
bool ProcessMessages(CNode* pfrom);
bool ProcessMessage(CNode* pfrom, string strCommand, CDataStream& vRecv);
bool SendMessages(CNode* pto);
int64 CountMoney();
bool CreateTransaction(CScript scriptPubKey, int64 nValue, CWalletTx& txNew);
bool SendMoney(CScript scriptPubKey, int64 nValue, CWalletTx& wtxNew);











class CDiskTxPos
{
public:
    unsigned int nFile;
    unsigned int nBlockPos;
    unsigned int nTxPos;

    CDiskTxPos()
    {
        SetNull();
    }

    CDiskTxPos(unsigned int nFileIn, unsigned int nBlockPosIn, unsigned int nTxPosIn)
    {
        nFile = nFileIn;
        nBlockPos = nBlockPosIn;
        nTxPos = nTxPosIn;
    }

    IMPLEMENT_SERIALIZE( READWRITE(FLATDATA(*this)); )
    void SetNull() { nFile = -1; nBlockPos = 0; nTxPos = 0; }
    bool IsNull() const { return (nFile == -1); }

    friend bool operator==(const CDiskTxPos& a, const CDiskTxPos& b)
    {
        return (a.nFile     == b.nFile &&
                a.nBlockPos == b.nBlockPos &&
                a.nTxPos    == b.nTxPos);
    }

    friend bool operator!=(const CDiskTxPos& a, const CDiskTxPos& b)
    {
        return !(a == b);
    }

    void print() const
    {
        if (IsNull())
            printf("null");
        else
            printf("(nFile=%d, nBlockPos=%d, nTxPos=%d)", nFile, nBlockPos, nTxPos);
    }
};




class CInPoint
{
public:
    CTransaction* ptx;
    unsigned int n;

    CInPoint() { SetNull(); }
    CInPoint(CTransaction* ptxIn, unsigned int nIn) { ptx = ptxIn; n = nIn; }
    void SetNull() { ptx = NULL; n = -1; }
    bool IsNull() const { return (ptx == NULL && n == -1); }
};




class COutPoint
{
public:
    uint256 hash;
    unsigned int n;

    COutPoint() { SetNull(); }
    COutPoint(uint256 hashIn, unsigned int nIn) { hash = hashIn; n = nIn; }
    IMPLEMENT_SERIALIZE( READWRITE(FLATDATA(*this)); )
    void SetNull() { hash = 0; n = -1; }
    bool IsNull() const { return (hash == 0 && n == -1); }

    friend bool operator<(const COutPoint& a, const COutPoint& b)
    {
        return (a.hash < b.hash || (a.hash == b.hash && a.n < b.n));
    }

    friend bool operator==(const COutPoint& a, const COutPoint& b)
    {
        return (a.hash == b.hash && a.n == b.n);
    }

    friend bool operator!=(const COutPoint& a, const COutPoint& b)
    {
        return !(a == b);
    }

    void print() const
    {
        printf("COutPoint(%s, %d)", hash.ToString().substr(0,6).c_str(), n);
    }
};




//
// An input of a transaction.  It contains the location of the previous
// transaction's output that it claims and a signature that matches the
// output's public key.
//
class CTxIn
{
public:
    COutPoint prevout;
    CScript scriptSig;

    CTxIn()
    {
    }

    CTxIn(COutPoint prevoutIn, CScript scriptSigIn)
    {
        prevout = prevoutIn;
        scriptSig = scriptSigIn;
    }

    CTxIn(uint256 hashPrevTx, unsigned int nOut, CScript scriptSigIn)
    {
        prevout = COutPoint(hashPrevTx, nOut);
        scriptSig = scriptSigIn;
    }

    IMPLEMENT_SERIALIZE
    (
        READWRITE(prevout);
        READWRITE(scriptSig);
    )

    bool IsPrevInMainChain() const
    {
        return CTxDB("r").ContainsTx(prevout.hash);
    }

    friend bool operator==(const CTxIn& a, const CTxIn& b)
    {
        return (a.prevout == b.prevout && a.scriptSig == b.scriptSig);
    }

    friend bool operator!=(const CTxIn& a, const CTxIn& b)
    {
        return !(a == b);
    }

    void print() const
    {
        printf("CTxIn(");
        prevout.print();
        if (prevout.IsNull())
        {
            printf(", coinbase %s)\n", HexStr(scriptSig.begin(), scriptSig.end(), false).c_str());
        }
        else
        {
            if (scriptSig.size() >= 6)
                printf(", scriptSig=%02x%02x", scriptSig[4], scriptSig[5]);
            printf(")\n");
        }
    }

    bool IsMine() const;
    int64 GetDebit() const;
};




//
// An output of a transaction.  It contains the public key that the next input
// must be able to sign with to claim it.
//
class CTxOut
{
public:
    int64 nValue;
    unsigned int nSequence;
    CScript scriptPubKey;

    // disk only
    CDiskTxPos posNext;  //// so far this is only used as a flag, nothing uses the location

public:
    CTxOut()
    {
        nValue = 0;
        nSequence = UINT_MAX;
    }

    CTxOut(int64 nValueIn, CScript scriptPubKeyIn, int nSequenceIn=UINT_MAX)
    {
        nValue = nValueIn;
        scriptPubKey = scriptPubKeyIn;
        nSequence = nSequenceIn;
    }

    IMPLEMENT_SERIALIZE
    (
        READWRITE(nValue);
        READWRITE(nSequence);
        READWRITE(scriptPubKey);
        if (nType & SER_DISK)
            READWRITE(posNext);
    )

    uint256 GetHash() const { return SerializeHash(*this); }

    bool IsFinal() const
    {
        return (nSequence == UINT_MAX);
    }

    bool IsMine() const
    {
        return ::IsMine(scriptPubKey);
    }

    int64 GetCredit() const
    {
        if (IsMine())
            return nValue;
        return 0;
    }

    friend bool operator==(const CTxOut& a, const CTxOut& b)
    {
        return (a.nValue       == b.nValue &&
                a.nSequence    == b.nSequence &&
                a.scriptPubKey == b.scriptPubKey);
    }

    friend bool operator!=(const CTxOut& a, const CTxOut& b)
    {
        return !(a == b);
    }

    void print() const
    {
        if (scriptPubKey.size() >= 6)
            printf("CTxOut(nValue=%I64d, nSequence=%u, scriptPubKey=%02x%02x, posNext=", nValue, nSequence, scriptPubKey[4], scriptPubKey[5]);
        posNext.print();
        printf(")\n");
    }
};




//
// The basic transaction that is broadcasted on the network and contained in
// blocks.  A transaction can contain multiple inputs and outputs.
//
class CTransaction
{
public:
    vector<CTxIn> vin;
    vector<CTxOut> vout;
    unsigned int nLockTime;


    CTransaction()
    {
        SetNull();
    }

    IMPLEMENT_SERIALIZE
    (
        if (!(nType & SER_GETHASH))
            READWRITE(nVersion);

        // Set version on stream for writing back same version
        if (fRead && s.nVersion == -1)
            s.nVersion = nVersion;

        READWRITE(vin);
        READWRITE(vout);
        READWRITE(nLockTime);
    )

    void SetNull()
    {
        vin.clear();
        vout.clear();
        nLockTime = 0;
    }

    bool IsNull() const
    {
        return (vin.empty() && vout.empty());
    }

    uint256 GetHash() const
    {
        return SerializeHash(*this);
    }

    bool AllPrevInMainChain() const
    {
        foreach(const CTxIn& txin, vin)
            if (!txin.IsPrevInMainChain())
                return false;
        return true;
    }

    bool IsFinal() const
    {
        if (nLockTime == 0)
            return true;
        if (nLockTime < GetAdjustedTime())
            return true;
        foreach(const CTxOut& txout, vout)
            if (!txout.IsFinal())
                return false;
        return true;
    }

    bool IsUpdate(const CTransaction& b) const
    {
        if (vin.size() != b.vin.size() || vout.size() != b.vout.size())
            return false;
        for (int i = 0; i < vin.size(); i++)
            if (vin[i].prevout != b.vin[i].prevout)
                return false;

        bool fNewer = false;
        unsigned int nLowest = UINT_MAX;
        for (int i = 0; i < vout.size(); i++)
        {
            if (vout[i].nSequence != b.vout[i].nSequence)
            {
                if (vout[i].nSequence <= nLowest)
                {
                    fNewer = false;
                    nLowest = vout[i].nSequence;
                }
                if (b.vout[i].nSequence < nLowest)
                {
                    fNewer = true;
                    nLowest = b.vout[i].nSequence;
                }
            }
        }
        return fNewer;
    }

    bool IsCoinBase() const
    {
        return (vin.size() == 1 && vin[0].prevout.IsNull());
    }

    bool CheckTransaction() const
    {
        // Basic checks that don't depend on any context
        if (vin.empty() || vout.empty())
            return false;

        // Check for negative values
        int64 nValueOut = 0;
        foreach(const CTxOut& txout, vout)
        {
            if (txout.nValue < 0)
                return false;
            nValueOut += txout.nValue;
        }

        if (IsCoinBase())
        {
            if (vin[0].scriptSig.size() > 100)
                return false;
        }
        else
        {
            foreach(const CTxIn& txin, vin)
                if (txin.prevout.IsNull())
                    return false;
        }

        return true;
    }

    bool IsMine() const
    {
        foreach(const CTxOut& txout, vout)
            if (txout.IsMine())
                return true;
        return false;
    }

    int64 GetDebit() const
    {
        int64 nDebit = 0;
        foreach(const CTxIn& txin, vin)
            nDebit += txin.GetDebit();
        return nDebit;
    }

    int64 GetCredit() const
    {
        int64 nCredit = 0;
        foreach(const CTxOut& txout, vout)
            nCredit += txout.GetCredit();
        return nCredit;
    }

    int64 GetValueOut() const
    {
        int64 nValueOut = 0;
        foreach(const CTxOut& txout, vout)
        {
            if (txout.nValue < 0)
                throw runtime_error("CTransaction::GetValueOut() : negative value");
            nValueOut += txout.nValue;
        }
        return nValueOut;
    }



    bool ReadFromDisk(CDiskTxPos pos, FILE** pfileRet=NULL)
    {
        CAutoFile filein = OpenBlockFile(pos.nFile, 0, pfileRet ? "rb+" : "rb");
        if (!filein)
            return false;

        // Read transaction
        if (fseek(filein, pos.nTxPos, SEEK_SET) != 0)
            return false;
        filein >> *this;

        // Return file pointer
        if (pfileRet)
        {
            if (fseek(filein, pos.nTxPos, SEEK_SET) != 0)
                return false;
            *pfileRet = filein.release();
        }
        return true;
    }


    friend bool operator==(const CTransaction& a, const CTransaction& b)
    {
        return (a.vin       == b.vin &&
                a.vout      == b.vout &&
                a.nLockTime == b.nLockTime);
    }

    friend bool operator!=(const CTransaction& a, const CTransaction& b)
    {
        return !(a == b);
    }


    void print() const
    {
        printf("CTransaction(vin.size=%d, vout.size=%d, nLockTime=%d)\n",
            vin.size(),
            vout.size(),
            nLockTime);
        for (int i = 0; i < vin.size(); i++)
        {
            printf("    ");
            vin[i].print();
        }
        for (int i = 0; i < vout.size(); i++)
        {
            printf("    ");
            vout[i].print();
        }
    }



    bool TestDisconnectInputs(CTxDB& txdb, map<uint256, CTransaction>& mapTestPool)
    {
        return DisconnectInputs(txdb, mapTestPool, true);
    }

    bool TestConnectInputs(CTxDB& txdb, map<uint256, CTransaction>& mapTestPool, bool fMemoryTx, bool fIgnoreDiskConflicts, int64& nFees)
    {
        return ConnectInputs(txdb, mapTestPool, CDiskTxPos(1, 1, 1), 0, true, fMemoryTx, fIgnoreDiskConflicts, nFees);
    }

    bool DisconnectInputs(CTxDB& txdb)
    {
        static map<uint256, CTransaction> mapTestPool;
        return DisconnectInputs(txdb, mapTestPool, false);
    }

    bool ConnectInputs(CTxDB& txdb, CDiskTxPos posThisTx, int nHeight)
    {
        static map<uint256, CTransaction> mapTestPool;
        int64 nFees;
        return ConnectInputs(txdb, mapTestPool, posThisTx, nHeight, false, false, false, nFees);
    }

private:
    bool DisconnectInputs(CTxDB& txdb, map<uint256, CTransaction>& mapTestPool, bool fTest);
    bool ConnectInputs(CTxDB& txdb, map<uint256, CTransaction>& mapTestPool, CDiskTxPos posThisTx, int nHeight,
                       bool fTest, bool fMemoryTx, bool fIgnoreDiskConflicts, int64& nFees);

public:
    bool AcceptTransaction(CTxDB& txdb, bool fCheckInputs=true);
    bool AcceptTransaction() { CTxDB txdb("r"); return AcceptTransaction(txdb); }
    bool ClientConnectInputs();
};





//
// A transaction with a merkle branch linking it to the timechain
//
class CMerkleTx : public CTransaction
{
public:
    uint256 hashBlock;
    vector<uint256> vMerkleBranch;
    int nIndex;

    CMerkleTx()
    {
        Init();
    }

    CMerkleTx(const CTransaction& txIn) : CTransaction(txIn)
    {
        Init();
    }

    void Init()
    {
        hashBlock = 0;
        nIndex = -1;
    }

    IMPLEMENT_SERIALIZE
    (
        nSerSize += SerReadWrite(s, *(CTransaction*)this, nType, nVersion, ser_action);
        if (!(nType & SER_GETHASH))
            READWRITE(nVersion);
        READWRITE(hashBlock);
        READWRITE(vMerkleBranch);
        READWRITE(nIndex);
    )


    int SetMerkleBranch();
    int IsInMainChain() const;
    bool AcceptTransaction(CTxDB& txdb, bool fCheckInputs=true);
    bool AcceptTransaction() { CTxDB txdb("r"); return AcceptTransaction(txdb); }
};




//
// A transaction with a bunch of additional info that only the owner cares
// about.  It includes any unrecorded transactions needed to link it back
// to the timechain.
//
class CWalletTx : public CMerkleTx
{
public:
    vector<CMerkleTx> vtxPrev;
    map<string, string> mapValue;
    vector<pair<string, string> > vOrderForm;
    unsigned int nTime;
    char fFromMe;
    char fSpent;

    //// probably need to sign the order info so know it came from payer

    CWalletTx()
    {
        Init();
    }

    CWalletTx(const CMerkleTx& txIn) : CMerkleTx(txIn)
    {
        Init();
    }

    CWalletTx(const CTransaction& txIn) : CMerkleTx(txIn)
    {
        Init();
    }

    void Init()
    {
        nTime = 0;
        fFromMe = false;
        fSpent = false;
    }

    IMPLEMENT_SERIALIZE
    (
        /// would be nice for it to return the version number it reads, maybe use a reference
        nSerSize += SerReadWrite(s, *(CMerkleTx*)this, nType, nVersion, ser_action);
        if (!(nType & SER_GETHASH))
            READWRITE(nVersion);
        READWRITE(vtxPrev);
        READWRITE(mapValue);
        READWRITE(vOrderForm);
        READWRITE(nTime);
        READWRITE(fFromMe);
        READWRITE(fSpent);
    )

    bool WriteToDisk()
    {
        return CWalletDB().WriteTx(GetHash(), *this);
    }


    void AddSupportingTransactions(CTxDB& txdb);
    void AddSupportingTransactions() { CTxDB txdb("r"); AddSupportingTransactions(txdb); }

    bool AcceptWalletTransaction(CTxDB& txdb, bool fCheckInputs=true);
    bool AcceptWalletTransaction() { CTxDB txdb("r"); return AcceptWalletTransaction(txdb); }

    void RelayWalletTransaction(CTxDB& txdb);
    void RelayWalletTransaction() { CTxDB txdb("r"); RelayWalletTransaction(txdb); }
};






//
// Nodes collect new transactions into a block, hash them into a hash tree,
// and scan through nonce values to make the block's hash satisfy proof-of-work
// requirements.  When they solve the proof-of-work, they broadcast the block
// to everyone and the block is added to the timechain.  The first transaction
// in the block is a special one that creates a new coin owned by the creator
// of the block.
//
// Blocks are appended to blk0001.dat files on disk.  Their location on disk
// is indexed by CBlockIndex objects in memory.
//
class CBlock
{
public:
    // header
    uint256 hashPrevBlock;
    uint256 hashMerkleRoot;
    unsigned int nTime;
    unsigned int nBits;
    unsigned int nNonce;

    // network and disk
    vector<CTransaction> vtx;

    // memory only
    mutable vector<uint256> vMerkleTree;


    CBlock()
    {
        SetNull();
    }

    IMPLEMENT_SERIALIZE
    (
        if (!(nType & SER_GETHASH))
            READWRITE(nVersion);
        READWRITE(hashPrevBlock);
        READWRITE(hashMerkleRoot);
        READWRITE(nTime);
        READWRITE(nBits);
        READWRITE(nNonce);

        // ConnectBlock depends on vtx being last so it can calculate offset
        if (!(nType & (SER_GETHASH|SER_BLOCKHEADERONLY)))
            READWRITE(vtx);
        else if (fRead)
            const_cast<CBlock*>(this)->vtx.clear();
    )

    void SetNull()
    {
        hashPrevBlock = 0;
        hashMerkleRoot = 0;
        nTime = 0;
        nBits = 0;
        nNonce = 0;
        vtx.clear();
        vMerkleTree.clear();
    }

    bool IsNull() const
    {
        return (nBits == 0);
    }

    uint256 GetHash() const
    {
        return Hash(BEGIN(hashPrevBlock), END(nNonce));
    }

    uint256 BuildMerkleTree() const
    {
        vMerkleTree.clear();
        foreach(const CTransaction& tx, vtx)
            vMerkleTree.push_back(tx.GetHash());
        int j = 0;
        for (int nSize = vtx.size(); nSize > 1; nSize = (nSize + 1) / 2)
        {
            for (int i = 0; i < nSize; i += 2)
            {
                int i2 = min(i+1, nSize-1);
                vMerkleTree.push_back(Hash(BEGIN(vMerkleTree[j+i]),  END(vMerkleTree[j+i]),
                                           BEGIN(vMerkleTree[j+i2]), END(vMerkleTree[j+i2])));
            }
            j += nSize;
        }
        return (vMerkleTree.empty() ? 0 : vMerkleTree.back());
    }

    vector<uint256> GetMerkleBranch(int nIndex) const
    {
        if (vMerkleTree.empty())
            BuildMerkleTree();
        vector<uint256> vMerkleBranch;
        int j = 0;
        for (int nSize = vtx.size(); nSize > 1; nSize = (nSize + 1) / 2)
        {
            int i = min(nIndex^1, nSize-1);
            vMerkleBranch.push_back(vMerkleTree[j+i]);
            nIndex >>= 1;
            j += nSize;
        }
        return vMerkleBranch;
    }

    static uint256 CheckMerkleBranch(uint256 hash, const vector<uint256>& vMerkleBranch, int nIndex)
    {
        foreach(const uint256& otherside, vMerkleBranch)
        {
            if (nIndex & 1)
                hash = Hash(BEGIN(otherside), END(otherside), BEGIN(hash), END(hash));
            else
                hash = Hash(BEGIN(hash), END(hash), BEGIN(otherside), END(otherside));
            nIndex >>= 1;
        }
        return hash;
    }


    bool WriteToDisk(bool fWriteTransactions, unsigned int& nFileRet, unsigned int& nBlockPosRet)
    {
        // Open history file to append
        CAutoFile fileout = AppendBlockFile(nFileRet);
        if (!fileout)
            return false;
        if (!fWriteTransactions)
            fileout.nType |= SER_BLOCKHEADERONLY;

        // Write index header
        unsigned int nSize = fileout.GetSerializeSize(*this);
        fileout << FLATDATA(pchMessageStart) << nSize;

        // Write block
        nBlockPosRet = ftell(fileout);
        if (nBlockPosRet == -1)
            return false;
        fileout << *this;

        return true;
    }

    bool ReadFromDisk(unsigned int nFile, unsigned int nBlockPos, bool fReadTransactions)
    {
        SetNull();

        // Open history file to read
        CAutoFile filein = OpenBlockFile(nFile, nBlockPos, "rb");
        if (!filein)
            return false;
        if (!fReadTransactions)
            filein.nType |= SER_BLOCKHEADERONLY;

        // Read block
        filein >> *this;

        // Check the header
        if (nBits < MINPROOFOFWORK || GetHash() > (~uint256(0) >> nBits))
            return error("CBlock::ReadFromDisk : errors in block header");

        return true;
    }



    void print() const
    {
        printf("CBlock(hashPrevBlock=%s, hashMerkleRoot=%s, nTime=%u, nBits=%u, nNonce=%u, vtx=%d)\n",
            hashPrevBlock.ToString().substr(0,6).c_str(),
            hashMerkleRoot.ToString().substr(0,6).c_str(),
            nTime, nBits, nNonce,
            vtx.size());
        for (int i = 0; i < vtx.size(); i++)
        {
            printf("  ");
            vtx[i].print();
        }
        printf("  vMerkleTree: ");
        for (int i = 0; i < vMerkleTree.size(); i++)
            printf("%s ", vMerkleTree[i].ToString().substr(0,6).c_str());
        printf("\n");
    }



    bool ReadFromDisk(const CBlockIndex* blockindex, bool fReadTransactions);
    bool TestDisconnectBlock(CTxDB& txdb, map<uint256, CTransaction>& mapTestPool);
    bool TestConnectBlock(CTxDB& txdb, map<uint256, CTransaction>& mapTestPool);
    bool DisconnectBlock();
    bool ConnectBlock(unsigned int nFile, unsigned int nBlockPos, int nHeight);
    bool AddToBlockIndex(unsigned int nFile, unsigned int nBlockPos, bool fWriteDisk);
    bool CheckBlock() const;
    bool AcceptBlock();
};






//
// The timechain is a tree shaped structure starting with the
// genesis block at the root, with each block potentially having multiple
// candidates to be the next block.  pprev and pnext link a path through the
// main/longest chain.  A blockindex may have multiple pprev pointing back
// to it, but pnext will only point forward to the longest branch, or will
// be null if the block is not part of the longest chain.
//
class CBlockIndex
{
public:
    CBlockIndex* pprev;
    CBlockIndex* pnext;
    unsigned int nFile;
    unsigned int nBlockPos;
    int nHeight;


    CBlockIndex()
    {
        pprev = NULL;
        pnext = NULL;
        nFile = 0;
        nBlockPos = 0;
        nHeight = 0;
    }

    CBlockIndex(unsigned int nFileIn, unsigned int nBlockPosIn)
    {
        pprev = NULL;
        pnext = NULL;
        nFile = nFileIn;
        nBlockPos = nBlockPosIn;
        nHeight = 0;
    }

    bool IsInMainChain() const
    {
        return (pnext || this == pindexBest);
    }

    bool EraseBlockFromDisk()
    {
        // Open history file
        CAutoFile fileout = OpenBlockFile(nFile, nBlockPos, "rb+");
        if (!fileout)
            return false;

        // Overwrite with empty null block
        CBlock block;
        block.SetNull();
        fileout << block;

        return true;
    }



    bool TestDisconnectBlock(CTxDB& txdb, map<uint256, CTransaction>& mapTestPool)
    {
        CBlock block;
        if (!block.ReadFromDisk(nFile, nBlockPos, true))
            return false;
        return block.TestDisconnectBlock(txdb, mapTestPool);
    }

    bool TestConnectBlock(CTxDB& txdb, map<uint256, CTransaction>& mapTestPool)
    {
        CBlock block;
        if (!block.ReadFromDisk(nFile, nBlockPos, true))
            return false;
        return block.TestConnectBlock(txdb, mapTestPool);
    }

    bool DisconnectBlock()
    {
        CBlock block;
        if (!block.ReadFromDisk(nFile, nBlockPos, true))
            return false;
        return block.DisconnectBlock();
    }

    bool ConnectBlock()
    {
        CBlock block;
        if (!block.ReadFromDisk(nFile, nBlockPos, true))
            return false;
        return block.ConnectBlock(nFile, nBlockPos, nHeight);
    }



    void print() const
    {
        printf("CBlockIndex(nprev=%08x, pnext=%08x, nFile=%d, nBlockPos=%d, nHeight=%d)\n",
            pprev, pnext, nFile, nBlockPos, nHeight);
    }
};

void PrintTimechain();







//
// Describes a place in the timechain to another node such that if the
// other node doesn't have the same branch, it can find a recent common trunk.
// The further back it is, the further before the branch point it may be.
//
class CBlockLocator
{
protected:
    vector<uint256> vHave;
public:

    CBlockLocator()
    {
    }

    explicit CBlockLocator(const CBlockIndex* pindex)
    {
        Set(pindex);
    }

    explicit CBlockLocator(uint256 hashBlock)
    {
        map<uint256, CBlockIndex*>::iterator mi = mapBlockIndex.find(hashBlock);
        if (mi != mapBlockIndex.end())
            Set((*mi).second);
    }

    IMPLEMENT_SERIALIZE
    (
        if (!(nType & SER_GETHASH))
            READWRITE(nVersion);
        READWRITE(vHave);
    )

    void Set(const CBlockIndex* pindex)
    {
        vHave.clear();
        int nStep = 1;
        while (pindex)
        {
            CBlock block;
            block.ReadFromDisk(pindex, false);
            vHave.push_back(block.GetHash());

            // Exponentially larger steps back
            for (int i = 0; pindex && i < nStep; i++)
                pindex = pindex->pprev;
            if (vHave.size() > 10)
                nStep *= 2;
        }
    }

    CBlockIndex* GetBlockIndex()
    {
        // Find the first block the caller has in the main chain
        foreach(const uint256& hash, vHave)
        {
            map<uint256, CBlockIndex*>::iterator mi = mapBlockIndex.find(hash);
            if (mi != mapBlockIndex.end())
            {
                CBlockIndex* pindex = (*mi).second;
                if (pindex->IsInMainChain())
                    return pindex;
            }
        }
        return pindexGenesisBlock;
    }

    uint256 GetBlockHash()
    {
        // Find the first block the caller has in the main chain
        foreach(const uint256& hash, vHave)
        {
            map<uint256, CBlockIndex*>::iterator mi = mapBlockIndex.find(hash);
            if (mi != mapBlockIndex.end())
            {
                CBlockIndex* pindex = (*mi).second;
                if (pindex->IsInMainChain())
                    return hash;
            }
        }
        return hashGenesisBlock;
    }

    int GetHeight()
    {
        CBlockIndex* pindex = GetBlockIndex();
        if (!pindex)
            return 0;
        return pindex->nHeight;
    }
};












extern map<uint256, CTransaction> mapTransactions;
extern map<uint256, CWalletTx> mapWallet;
extern vector<pair<uint256, bool> > vWalletUpdated;
extern CCriticalSection cs_mapWallet;
extern map<vector<unsigned char>, CPrivKey> mapKeys;
extern map<uint160, vector<unsigned char> > mapPubKeys;
extern CCriticalSection cs_mapKeys;
extern CKey keyUser;






//////////////////////////////////////////////////////////////////////////////
//
// Messages
//


bool AlreadyHave(const CInv& inv)
{
    switch (inv.type)
    {
    case MSG_TX:        return mapTransactions.count(inv.hash);
    case MSG_BLOCK:     return mapBlockIndex.count(inv.hash) || mapOrphanBlocks.count(inv.hash);
    case MSG_REVIEW:    return true;
    case MSG_PRODUCT:   return mapProducts.count(inv.hash);
    case MSG_TABLE:     return mapTables.count(inv.hash);
    }
    // Don't know what it is, just say we already got one
    return true;
}







bool ProcessMessages(CNode* pfrom)
{
    CDataStream& vRecv = pfrom->vRecv;
    if (vRecv.empty())
        return true;
    printf("ProcessMessages(%d bytes)\n", vRecv.size());

    //
    // Message format
    //  (4) message start
    //  (12) command
    //  (4) size
    //  (x) data
    //

    loop
    {
        // Scan for message start
        CDataStream::iterator pstart = search(vRecv.begin(), vRecv.end(), BEGIN(pchMessageStart), END(pchMessageStart));
        if (vRecv.end() - pstart < sizeof(CMessageHeader))
        {
            if (vRecv.size() > sizeof(CMessageHeader))
            {
                printf("\n\nPROCESSMESSAGE MESSAGESTART NOT FOUND\n\n");
                vRecv.erase(vRecv.begin(), vRecv.end() - sizeof(CMessageHeader));
            }
            break;
        }
        if (pstart - vRecv.begin() > 0)
            printf("\n\nPROCESSMESSAGE SKIPPED %d BYTES\n\n", pstart - vRecv.begin());
        vRecv.erase(vRecv.begin(), pstart);

        // Read header
        CMessageHeader hdr;
        vRecv >> hdr;
        if (!hdr.IsValid())
        {
            printf("\n\nPROCESSMESSAGE: ERRORS IN HEADER %s\n\n\n", hdr.GetCommand().c_str());
            continue;
        }
        string strCommand = hdr.GetCommand();

        // Message size
        unsigned int nMessageSize = hdr.nMessageSize;
        if (nMessageSize > vRecv.size())
        {
            // Rewind and wait for rest of message
            ///// need a mechanism to give up waiting for overlong message size error
            printf("MESSAGE-BREAK 2\n");
            vRecv.insert(vRecv.begin(), BEGIN(hdr), END(hdr));
            break;
        }

        // Copy message to its own buffer
        CDataStream vMsg(vRecv.begin(), vRecv.begin() + nMessageSize, vRecv.nType, vRecv.nVersion);
        vRecv.ignore(nMessageSize);

        // Process message
        bool fRet = false;
        try
        {
            fRet = ProcessMessage(pfrom, strCommand, vMsg);
        }
        CATCH_PRINT_EXCEPTION("ProcessMessage()")
        if (!fRet)
            printf("ProcessMessage(%s, %d bytes) from %s to %s FAILED\n", strCommand.c_str(), nMessageSize, pfrom->addr.ToString().c_str(), addrLocalHost.ToString().c_str());
    }

    vRecv.Compact();
    return true;
}




bool ProcessMessage(CNode* pfrom, string strCommand, CDataStream& vRecv)
{
    static map<unsigned int, vector<unsigned char> > mapReuseKey;
    CheckForShutdown(2);
    printf("received: %-12s (%d bytes)  ", strCommand.c_str(), vRecv.size());
    for (int i = 0; i < min(vRecv.size(), (unsigned int)25); i++)
        printf("%02x ", vRecv[i] & 0xff);
    printf("\n");


    if (strCommand == "version")
    {
        // Can only do this once
        if (pfrom->nVersion != 0)
            return false;

        unsigned int nTime;
        vRecv >> pfrom->nVersion >> pfrom->nServices >> nTime;
        if (pfrom->nVersion == 0)
            return false;

        pfrom->vSend.SetVersion(min(pfrom->nVersion, VERSION));
        pfrom->vRecv.SetVersion(min(pfrom->nVersion, VERSION));

        pfrom->fClient = !(pfrom->nServices & NODE_NETWORK);
        if (pfrom->fClient)
        {
            pfrom->vSend.nType |= SER_BLOCKHEADERONLY;
            pfrom->vRecv.nType |= SER_BLOCKHEADERONLY;
        }

        AddTimeData(pfrom->addr.ip, nTime);

        // Ask the first connected node for block updates
        static bool fAskedForBlocks;
        if (!fAskedForBlocks && !pfrom->fClient)
        {
            fAskedForBlocks = true;
            pfrom->PushMessage("getblocks", CBlockLocator(pindexBest), uint256(0));
        }
    }


    else if (pfrom->nVersion == 0)
    {
        // Must have a version message before anything else
        return false;
    }


    else if (strCommand == "addr")
    {
        vector<CAddress> vAddr;
        vRecv >> vAddr;

        // Store the new addresses
        CAddrDB addrdb;
        foreach(const CAddress& addr, vAddr)
        {
            if (AddAddress(addrdb, addr))
            {
                // Put on lists to send to other nodes
                pfrom->setAddrKnown.insert(addr);
                CRITICAL_BLOCK(cs_vNodes)
                    foreach(CNode* pnode, vNodes)
                        if (!pnode->setAddrKnown.count(addr))
                            pnode->vAddrToSend.push_back(addr);
            }
        }
    }


    else if (strCommand == "inv")
    {
        vector<CInv> vInv;
        vRecv >> vInv;

        foreach(const CInv& inv, vInv)
        {
            printf("  got inventory: %s  %s\n", inv.ToString().c_str(), AlreadyHave(inv) ? "have" : "new");

            CRITICAL_BLOCK(pfrom->cs_inventory)
                pfrom->setInventoryKnown.insert(inv);

            if (!AlreadyHave(inv))
                pfrom->AskFor(inv);
            else if (inv.type == MSG_BLOCK && mapOrphanBlocks.count(inv.hash))
                pfrom->PushMessage("getblocks", CBlockLocator(pindexBest), GetOrphanRoot(mapOrphanBlocks[inv.hash]));
        }
    }


    else if (strCommand == "getdata")
    {
        vector<CInv> vInv;
        vRecv >> vInv;

        foreach(const CInv& inv, vInv)
        {
            printf("received getdata for: %s\n", inv.ToString().c_str());

            if (inv.type == MSG_BLOCK)
            {
                // Send block from disk
                map<uint256, CBlockIndex*>::iterator mi = mapBlockIndex.find(inv.hash);
                if (mi != mapBlockIndex.end())
                {
                    CBlock block;
                    block.ReadFromDisk((*mi).second, !pfrom->fClient);
                    pfrom->PushMessage("block", block);
                }
            }
            else if (inv.IsKnownType())
            {
                // Send stream from relay memory
                CRITICAL_BLOCK(cs_mapRelay)
                {
                    map<CInv, CDataStream>::iterator mi = mapRelay.find(inv);
                    if (mi != mapRelay.end())
                        pfrom->PushMessage(inv.GetCommand(), (*mi).second);
                }
            }
        }
    }


    else if (strCommand == "getblocks")
    {
        CBlockLocator locator;
        uint256 hashStop;
        vRecv >> locator >> hashStop;

        // Find the first block the caller has in the main chain
        CBlockIndex* pindex = locator.GetBlockIndex();

        // Send the rest of the chain
        if (pindex)
            pindex = pindex->pnext;
        for (; pindex; pindex = pindex->pnext)
        {
            CBlock block;
            block.ReadFromDisk(pindex, !pfrom->fClient);
            if (block.GetHash() == hashStop)
                break;
            pfrom->PushMessage("block", block);
        }
    }


    else if (strCommand == "getmywtxes")
    {
        CBlockLocator locator;
        vector<uint160> vPubKeyHashes;
        vRecv >> locator >> vPubKeyHashes;

        // Find the owner's new transactions
        int nHeight = locator.GetHeight();
        CTxDB txdb("r");
        foreach(uint160 hash160, vPubKeyHashes)
        {
            vector<CTransaction> vtx;
            if (txdb.ReadOwnerTxes(hash160, nHeight, vtx))
            {
                foreach(const CTransaction& tx, vtx)
                {
                    // Upgrade transaction to a fully supported CWalletTx
                    CWalletTx wtx(tx);
                    wtx.AddSupportingTransactions(txdb);

                    pfrom->PushMessage("wtx", wtx);
                }
            }
        }
    }


    else if (strCommand == "wtx")
    {
        CWalletTx wtx;
        vRecv >> wtx;

        if (!wtx.AcceptWalletTransaction())
            return error("message wtx : AcceptWalletTransaction failed!");
        AddToWallet(wtx);
    }


    else if (strCommand == "tx")
    {
        CDataStream vMsg(vRecv);
        CTransaction tx;
        vRecv >> tx;

        CInv inv(MSG_TX, tx.GetHash());
        pfrom->AddInventoryKnown(inv);

        if (tx.AcceptTransaction())
        {
            AddToWalletIfMine(tx, NULL);
            RelayMessage(inv, vMsg);
            mapAlreadyAskedFor.erase(inv);
        }
    }


    else if (strCommand == "block")
    {
        auto_ptr<CBlock> pblock(new CBlock);
        vRecv >> *pblock;

        //// debug print
        printf("received block:\n"); pblock->print();

        CInv inv(MSG_BLOCK, pblock->GetHash());
        pfrom->AddInventoryKnown(inv);

        if (ProcessBlock(pfrom, pblock.release()))
            mapAlreadyAskedFor.erase(inv);
    }


    else if (strCommand == "getaddr")
    {
        pfrom->vAddrToSend.clear();
        //// need to expand the time range if not enough found
        int64 nSince = GetAdjustedTime() - 60 * 60; // in the last hour
        CRITICAL_BLOCK(cs_mapAddresses)
        {
            foreach(const PAIRTYPE(vector<unsigned char>, CAddress)& item, mapAddresses)
            {
                const CAddress& addr = item.second;
                if (addr.nTime > nSince)
                    pfrom->vAddrToSend.push_back(addr);
            }
        }
    }


    else if (strCommand == "checkorder")
    {
        uint256 hashReply;
        CWalletTx order;
        vRecv >> hashReply >> order;

        /// we have a chance to check the order here

        // Keep giving the same key to the same ip until they use it
        if (!mapReuseKey.count(pfrom->addr.ip))
            mapReuseKey[pfrom->addr.ip] = GenerateNewKey();

        // Send back approval of order and pubkey to use
        CScript scriptPubKey;
        scriptPubKey << OP_CODESEPARATOR << mapReuseKey[pfrom->addr.ip] << OP_CHECKSIG;
        pfrom->PushMessage("reply", hashReply, (int)0, scriptPubKey);
    }


    else if (strCommand == "submitorder")
    {
        uint256 hashReply;
        CWalletTx wtxNew;
        vRecv >> hashReply >> wtxNew;

        // Broadcast
        if (!wtxNew.AcceptWalletTransaction())
        {
            pfrom->PushMessage("reply", hashReply, (int)1);
            return error("submitorder AcceptWalletTransaction() failed, returning error 1");
        }
        AddToWallet(wtxNew);
        wtxNew.RelayWalletTransaction();
        mapReuseKey.erase(pfrom->addr.ip);

        // Send back confirmation
        pfrom->PushMessage("reply", hashReply, (int)0);
    }


    else if (strCommand == "reply")
    {
        uint256 hashReply;
        vRecv >> hashReply;

        CRequestTracker tracker;
        CRITICAL_BLOCK(pfrom->cs_mapRequests)
        {
            map<uint256, CRequestTracker>::iterator mi = pfrom->mapRequests.find(hashReply);
            if (mi != pfrom->mapRequests.end())
            {
                tracker = (*mi).second;
                pfrom->mapRequests.erase(mi);
            }
        }
        if (!tracker.IsNull())
            tracker.fn(tracker.param1, vRecv);
    }


    else
    {
        // Ignore unknown commands for extensibility
        printf("ProcessMessage(%s) : Ignored unknown message\n", strCommand.c_str());
    }


    if (!vRecv.empty())
        printf("ProcessMessage(%s) : %d extra bytes\n", strCommand.c_str(), vRecv.size());

    return true;
}









bool SendMessages(CNode* pto)
{
    CheckForShutdown(2);

    // Don't send anything until we get their version message
    if (pto->nVersion == 0)
        return true;


    //
    // Message: addr
    //
    vector<CAddress> vAddrToSend;
    vAddrToSend.reserve(pto->vAddrToSend.size());
    foreach(const CAddress& addr, pto->vAddrToSend)
        if (!pto->setAddrKnown.count(addr))
            vAddrToSend.push_back(addr);
    pto->vAddrToSend.clear();
    if (!vAddrToSend.empty())
        pto->PushMessage("addr", vAddrToSend);


    //
    // Message: inventory
    //
    vector<CInv> vInventoryToSend;
    CRITICAL_BLOCK(pto->cs_inventory)
    {
        vInventoryToSend.reserve(pto->vInventoryToSend.size());
        foreach(const CInv& inv, pto->vInventoryToSend)
            if (!pto->setInventoryKnown.count(inv))
                vInventoryToSend.push_back(inv);
        pto->vInventoryToSend.clear();
    }
    if (!vInventoryToSend.empty())
        pto->PushMessage("inv", vInventoryToSend);


    //
    // Message: getdata
    //
    vector<CInv> vAskFor;
    int64 nNow = GetTime();
    while (!pto->mapAskFor.empty() && (*pto->mapAskFor.begin()).first <= nNow)
    {
        const CInv& inv = (*pto->mapAskFor.begin()).second;
        printf("getdata %s\n", inv.ToString().c_str());
        if (!AlreadyHave(inv))
            vAskFor.push_back(inv);
        pto->mapAskFor.erase(pto->mapAskFor.begin());
    }
    if (!vAskFor.empty())
        pto->PushMessage("getdata", vAskFor);



    return true;
}














//////////////////////////////////////////////////////////////////////////////
//
// BitcoinMiner
//

int FormatHashBlocks(void* pbuffer, unsigned int len)
{
    unsigned char* pdata = (unsigned char*)pbuffer;
    unsigned int blocks = 1 + ((len + 8) / 64);
    unsigned char* pend = pdata + 64 * blocks;
    memset(pdata + len, 0, 64 * blocks - len);
    pdata[len] = 0x80;
    unsigned int bits = len * 8;
    pend[-1] = (bits >> 0) & 0xff;
    pend[-2] = (bits >> 8) & 0xff;
    pend[-3] = (bits >> 16) & 0xff;
    pend[-4] = (bits >> 24) & 0xff;
    return blocks;
}

using CryptoPP::ByteReverse;
static int detectlittleendian = 1;

void BlockSHA256(const void* pin, unsigned int nBlocks, void* pout)
{
    unsigned int* pinput = (unsigned int*)pin;
    unsigned int* pstate = (unsigned int*)pout;

    CryptoPP::SHA256::InitState(pstate);

    if (*(char*)&detectlittleendian != 0)
    {
        for (int n = 0; n < nBlocks; n++)
        {
            unsigned int pbuf[16];
            for (int i = 0; i < 16; i++)
                pbuf[i] = ByteReverse(pinput[n * 16 + i]);
            CryptoPP::SHA256::Transform(pstate, pbuf);
        }
        for (int i = 0; i < 8; i++)
            pstate[i] = ByteReverse(pstate[i]);
    }
    else
    {
        for (int n = 0; n < nBlocks; n++)
            CryptoPP::SHA256::Transform(pstate, pinput + n * 16);
    }
}


bool BitcoinMiner()
{
    printf("BitcoinMiner started\n");

    SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_LOWEST);



    CBlock blockPrev;
    while (fGenerateBitcoins)
    {
        CheckForShutdown(3);

        //
        // Create coinbase tx
        //
        CTransaction txNew;
        txNew.vin.resize(1);
        txNew.vin[0].prevout.SetNull();
        CBigNum bnNonce; // this nonce is so multiple processes working for the same keyUser
        BN_rand_range(&bnNonce, &CBigNum(INT_MAX));  // don't cover the same ground
        txNew.vin[0].scriptSig << bnNonce;
        txNew.vout.resize(1);
        txNew.vout[0].scriptPubKey << OP_CODESEPARATOR << keyUser.GetPubKey() << OP_CHECKSIG;
        txNew.vout[0].posNext.SetNull();


        //
        // Create new block
        //
        auto_ptr<CBlock> pblock(new CBlock());
        if (!pblock.get())
            return false;

        // Add our coinbase tx as first transaction
        pblock->vtx.push_back(txNew);

        // Collect the latest transactions into the block
        unsigned int nTransactionsUpdatedLast = nTransactionsUpdated;
        int64 nFees = 0;
        CRITICAL_BLOCK(cs_mapTransactions)
        {
            CTxDB txdb("r");
            set<uint256> setInThisBlock;
            vector<char> vfAlreadyAdded(mapTransactions.size());
            bool fFoundSomething = true;
            unsigned int nSize = 0;
            while (fFoundSomething && nSize < MAX_SIZE/2)
            {
                fFoundSomething = false;
                unsigned int n = 0;
                for (map<uint256, CTransaction>::iterator mi = mapTransactions.begin(); mi != mapTransactions.end(); ++mi, ++n)
                {
                    if (vfAlreadyAdded[n])
                        continue;
                    CTransaction& tx = (*mi).second;
                    if (!tx.IsFinal() || tx.IsCoinBase())
                        continue;

                    // Find if all dependencies are in this or previous blocks
                    bool fHaveAllPrev = true;
                    int64 nValueIn = 0;
                    foreach(const CTxIn& txin, tx.vin)
                    {
                        COutPoint prevout = txin.prevout;
                        CTransaction txPrev;
                        if (setInThisBlock.count(prevout.hash))
                        {
                            txPrev = mapTransactions[prevout.hash];
                        }
                        else if (!txdb.ReadDiskTx(prevout.hash, txPrev))
                        {
                            fHaveAllPrev = false;
                            break;
                        }
                        if (prevout.n >= txPrev.vout.size())
                        {
                            fHaveAllPrev = false;
                            break;
                        }
                        nValueIn += txPrev.vout[prevout.n].nValue;
                    }
                    int64 nTransactionFee = nValueIn - tx.GetValueOut();
                    if (nTransactionFee < 0) // could require a tx fee here
                        continue;

                    // Add tx to block
                    if (fHaveAllPrev)
                    {
                        fFoundSomething = true;
                        pblock->vtx.push_back(tx);
                        nSize += ::GetSerializeSize(tx, SER_NETWORK);
                        nFees += nTransactionFee;
                        vfAlreadyAdded[n] = true;
                        setInThisBlock.insert(tx.GetHash());
                    }
                }
            }
        }

        // Update last few things
        pblock->vtx[0].vout[0].nValue = GetBlockValue(nFees);
        pblock->hashMerkleRoot = pblock->BuildMerkleTree();


        printf("\n\nRunning BitcoinMiner with %d transactions in block\n", pblock->vtx.size());


        //
        // Prebuild hash buffer
        //
        struct unnamed1
        {
            struct unnamed2
            {
                uint256 hashPrevBlock;
                uint256 hashMerkleRoot;
                unsigned int nTime;
                unsigned int nBits;
                unsigned int nNonce;
            }
            block;
            unsigned char pchPadding0[64];
            uint256 hash1;
            unsigned char pchPadding1[64];
        }
        tmp;

        const CBlockIndex* pindexPrev = pindexBest;
        tmp.block.hashPrevBlock = pblock->hashPrevBlock = hashTimeChainBest;
        tmp.block.hashMerkleRoot = pblock->hashMerkleRoot;

        // Get time of previous block
        if (pindexPrev)
        {
            if (blockPrev.GetHash() != pblock->hashPrevBlock)
                blockPrev.ReadFromDisk(pindexPrev, false);
            if (blockPrev.GetHash() != pblock->hashPrevBlock)
            {
                printf("pindexBest and hashTimeChainBest out of sync\n");
                continue;
            }
        }
        tmp.block.nTime = pblock->nTime = max(blockPrev.nTime+1, (unsigned int)GetAdjustedTime());
        tmp.block.nBits = pblock->nBits = GetNextWorkRequired(pindexPrev);
        tmp.block.nNonce = 1;

        unsigned int nBlocks0 = FormatHashBlocks(&tmp.block, sizeof(tmp.block));
        unsigned int nBlocks1 = FormatHashBlocks(&tmp.hash1, sizeof(tmp.hash1));


        //
        // Search
        //
        uint256 hashTarget = (~uint256(0) >> pblock->nBits);
        uint256 hash;
        while (nTransactionsUpdated == nTransactionsUpdatedLast)
        {
            BlockSHA256(&tmp.block, nBlocks0, &tmp.hash1);
            BlockSHA256(&tmp.hash1, nBlocks1, &hash);

            if (hash <= hashTarget)
            {
                pblock->nNonce = tmp.block.nNonce;
                assert(hash == pblock->GetHash());

                    //// debug print
                    printf("BitcoinMiner:\n");
                    printf("supercoin found  \n  hash: %s  \ntarget: %s\n", hash.GetHex().c_str(), hashTarget.GetHex().c_str());
                    pblock->print();

                // Process this block the same as if we had received it from another node
                if (!ProcessBlock(NULL, pblock.release()))
                    printf("ERROR in BitcoinMiner, ProcessBlock, block not accepted\n");
                break;
            }

            // Update nTime every few seconds
            if ((++tmp.block.nNonce & 0xfffff) == 0)
            {
                if (tmp.block.nNonce == 0)
                    break;
                tmp.block.nTime = pblock->nTime = max(blockPrev.nTime+1, (unsigned int)GetAdjustedTime());
            }
        }
    }

    return true;
}


















//////////////////////////////////////////////////////////////////////////////
//
// Actions
//


int64 CountMoney()
{
    int64 nTotal = 0;
    CRITICAL_BLOCK(cs_mapWallet)
    {
        for (map<uint256, CWalletTx>::iterator it = mapWallet.begin(); it != mapWallet.end(); ++it)
        {
            CWalletTx* pcoin = &(*it).second;
            if (!pcoin->IsFinal() || pcoin->fSpent)
                continue;
            nTotal += pcoin->GetCredit();
        }
    }
    return nTotal;
}



bool SelectCoins(int64 nTargetValue, set<CWalletTx*>& setCoinsRet)
{
    setCoinsRet.clear();

    // List of values less than target
    int64 nLowestLarger = _I64_MAX;
    CWalletTx* pcoinLowestLarger = NULL;
    vector<pair<int64, CWalletTx*> > vValue;
    int64 nTotalLower = 0;

    CRITICAL_BLOCK(cs_mapWallet)
    {
        for (map<uint256, CWalletTx>::iterator it = mapWallet.begin(); it != mapWallet.end(); ++it)
        {
            CWalletTx* pcoin = &(*it).second;
            if (!pcoin->IsFinal() || pcoin->fSpent)
                continue;
            int64 n = pcoin->GetCredit();
            if (n < nTargetValue)
            {
                vValue.push_back(make_pair(n, pcoin));
                nTotalLower += n;
            }
            else if (n == nTargetValue)
            {
                setCoinsRet.insert(pcoin);
                return true;
            }
            else if (n < nLowestLarger)
            {
                nLowestLarger = n;
                pcoinLowestLarger = pcoin;
            }
        }
    }

    if (nTotalLower < nTargetValue)
    {
        if (pcoinLowestLarger == NULL)
            return false;
        setCoinsRet.insert(pcoinLowestLarger);
        return true;
    }

    // Solve subset sum by stochastic approximation
    sort(vValue.rbegin(), vValue.rend());
    vector<char> vfIncluded;
    vector<char> vfBest(vValue.size(), true);
    int64 nBest = nTotalLower;

    for (int nRep = 0; nRep < 1000 && nBest != nTargetValue; nRep++)
    {
        vfIncluded.assign(vValue.size(), false);
        int64 nTotal = 0;
        for (int i = 0; i < vValue.size(); i++)
        {
            if (rand() % 2)
            {
                nTotal += vValue[i].first;
                vfIncluded[i] = true;
                if (nTotal >= nTargetValue)
                {
                    if (nTotal < nBest)
                    {
                        nBest = nTotal;
                        vfBest = vfIncluded;
                    }
                    nTotal -= vValue[i].first;
                    vfIncluded[i] = false;
                }
            }
        }
    }

    // If the next larger is still closer, return it
    if (pcoinLowestLarger && nLowestLarger - nTargetValue <= nBest - nTargetValue)
        setCoinsRet.insert(pcoinLowestLarger);
    else
        for (int i = 0; i < vValue.size(); i++)
            if (vfBest[i])
                setCoinsRet.insert(vValue[i].second);
    return true;
}



bool CreateTransaction(CScript scriptPubKey, int64 nValue, CWalletTx& wtxNew)
{
    wtxNew.vin.clear();
    wtxNew.vout.clear();
    if (nValue < TRANSACTIONFEE)
        return false;

    // Choose coins to use
    set<CWalletTx*> setCoins;
    if (!SelectCoins(nValue, setCoins))
        return false;
    int64 nValueIn = 0;
    foreach(CWalletTx* pcoin, setCoins)
        nValueIn += pcoin->GetCredit();

    // Fill vout[0] to the payee
    int64 nValueOut = nValue - TRANSACTIONFEE;
    wtxNew.vout.push_back(CTxOut(nValueOut, scriptPubKey));

    // Fill vout[1] back to self with any change
    if (nValueIn - TRANSACTIONFEE > nValueOut)
    {
        // Use the same key as one of the coins
        vector<unsigned char> vchPubKey;
        CTransaction& txFirst = *(*setCoins.begin());
        foreach(const CTxOut& txout, txFirst.vout)
            if (txout.IsMine())
                if (ExtractPubKey(txout.scriptPubKey, true, vchPubKey))
                    break;
        if (vchPubKey.empty())
            return false;

        // Fill vout[1] to ourself
        CScript scriptPubKey;
        scriptPubKey << OP_CODESEPARATOR << vchPubKey << OP_CHECKSIG;
        wtxNew.vout.push_back(CTxOut(nValueIn - TRANSACTIONFEE - nValueOut, scriptPubKey));
    }

    // Fill vin
    foreach(CWalletTx* pcoin, setCoins)
        for (int nOut = 0; nOut < pcoin->vout.size(); nOut++)
            if (pcoin->vout[nOut].IsMine())
                SignSignature(*pcoin, nOut, wtxNew, -1, "all");

    // Fill vtxPrev by copying from previous transactions vtxPrev
    wtxNew.AddSupportingTransactions();

    // Add tx to wallet, because if it has change it's also ours,
    // otherwise just for transaction history.
    wtxNew.nTime = GetAdjustedTime();
    AddToWallet(wtxNew);

    // Mark old coins as spent
    foreach(CWalletTx* pcoin, setCoins)
    {
        pcoin->fSpent = true;
        pcoin->WriteToDisk();
        vWalletUpdated.push_back(make_pair(pcoin->GetHash(), false));
    }

    return true;
}



bool SendMoney(CScript scriptPubKey, int64 nValue, CWalletTx& wtxNew)
{
    if (!CreateTransaction(scriptPubKey, nValue, wtxNew))
        return false;

    // Broadcast
    if (!wtxNew.AcceptTransaction())
    {
        // This must not fail. The transaction has already been signed and recorded.
        throw runtime_error("SendMoney() : wtxNew.AcceptTransaction() failed\n");
        return false;
    }
    wtxNew.RelayWalletTransaction();

    return true;
}
