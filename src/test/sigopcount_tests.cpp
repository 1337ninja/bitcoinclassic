// Copyright (c) 2012-2015 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "pubkey.h"
#include "key.h"
#include "policy/policy.h"
#include "script/script.h"
#include "script/standard.h"
#include "uint256.h"
#include "test/test_bitcoin.h"

#include <vector>

#include <boost/foreach.hpp>
#include <boost/test/unit_test.hpp>

using namespace std;

// Helpers:
static std::vector<unsigned char>
Serialize(const CScript& s)
{
    std::vector<unsigned char> sSerialized(s.begin(), s.end());
    return sSerialized;
}

BOOST_FIXTURE_TEST_SUITE(sigopcount_tests, BasicTestingSetup)

BOOST_AUTO_TEST_CASE(GetSigOpCount)
{
    // Test CScript::GetSigOpCount()
    CScript s1;
    BOOST_CHECK_EQUAL(s1.GetSigOpCount(false), 0U);
    BOOST_CHECK_EQUAL(s1.GetSigOpCount(true), 0U);

    uint160 dummy;
    s1 << OP_1 << ToByteVector(dummy) << ToByteVector(dummy) << OP_2 << OP_CHECKMULTISIG;
    BOOST_CHECK_EQUAL(s1.GetSigOpCount(true), 2U);
    s1 << OP_IF << OP_CHECKSIG << OP_ENDIF;
    BOOST_CHECK_EQUAL(s1.GetSigOpCount(true), 3U);
    BOOST_CHECK_EQUAL(s1.GetSigOpCount(false), 21U);

    CScript p2sh = GetScriptForDestination(CScriptID(s1));
    CScript scriptSig;
    scriptSig << OP_0 << Serialize(s1);
    BOOST_CHECK_EQUAL(p2sh.GetSigOpCount(scriptSig), 3U);

    std::vector<CPubKey> keys;
    for (int i = 0; i < 3; i++)
    {
        CKey k;
        k.MakeNewKey(true);
        keys.push_back(k.GetPubKey());
    }
    CScript s2 = GetScriptForMultisig(1, keys);
    BOOST_CHECK_EQUAL(s2.GetSigOpCount(true), 3U);
    BOOST_CHECK_EQUAL(s2.GetSigOpCount(false), 20U);

    p2sh = GetScriptForDestination(CScriptID(s2));
    BOOST_CHECK_EQUAL(p2sh.GetSigOpCount(true), 0U);
    BOOST_CHECK_EQUAL(p2sh.GetSigOpCount(false), 0U);
    CScript scriptSig2;
    scriptSig2 << OP_1 << ToByteVector(dummy) << ToByteVector(dummy) << Serialize(s2);
    BOOST_CHECK_EQUAL(p2sh.GetSigOpCount(scriptSig2), 3U);
}


BOOST_AUTO_TEST_CASE(blockSigOpAcceptLimit)
{
    BOOST_CHECK_EQUAL(Policy::blockSigOpAcceptLimit(0), 20000);
    BOOST_CHECK_EQUAL(Policy::blockSigOpAcceptLimit(1), 20000);
    BOOST_CHECK_EQUAL(Policy::blockSigOpAcceptLimit(70000), 20000);
    BOOST_CHECK_EQUAL(Policy::blockSigOpAcceptLimit(999999), 20000);
    BOOST_CHECK_EQUAL(Policy::blockSigOpAcceptLimit(1000000), 20000);
    BOOST_CHECK_EQUAL(Policy::blockSigOpAcceptLimit(1000001), 40000);
    BOOST_CHECK_EQUAL(Policy::blockSigOpAcceptLimit(1700000), 40000);
    BOOST_CHECK_EQUAL(Policy::blockSigOpAcceptLimit(1999999), 40000);
    BOOST_CHECK_EQUAL(Policy::blockSigOpAcceptLimit(2000000), 40000);
    BOOST_CHECK_EQUAL(Policy::blockSigOpAcceptLimit(2000001), 60000);
    BOOST_CHECK_EQUAL(Policy::blockSigOpAcceptLimit(INT32_MAX), 42960000);
}

BOOST_AUTO_TEST_SUITE_END()
