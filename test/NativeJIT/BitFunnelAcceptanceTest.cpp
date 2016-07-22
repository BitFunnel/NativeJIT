// The MIT License (MIT)

// Copyright (c) 2016, Microsoft

// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:

// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.

// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.


#include <cmath>        // For float std::abs(float).
#include <iostream>
#include <map>

#include "NativeJIT/CodeGen/ExecutionBuffer.h"
#include "NativeJIT/CodeGen/FunctionBuffer.h"
#include "NativeJIT/Function.h"
#include "NativeJIT/Model.h"
#include "NativeJIT/Packed.h"
#include "TestSetup.h"


namespace NativeJIT
{
    namespace BitFunnelAcceptanceTest
    {
        TEST_FIXTURE_START(Acceptance)

        public:
            Acceptance() : TestFixture(5000, 64 * 1024, TestFixture::c_defaultDiagnosticsStream)
            {
            }

        protected:
            static const unsigned c_bitsForAnchor = 4;
            static const unsigned c_bitsForBody = 4;
            static const unsigned c_bitsForTitle = 1;
            static const unsigned c_bitsForUrl = 1;
            static const unsigned c_bitsForPosition = 4;
            static const unsigned c_bitsForShard = 4;

            typedef Packed<c_bitsForAnchor, c_bitsForBody, c_bitsForTitle, c_bitsForUrl> TermFrequencies;
            typedef Packed<c_bitsForAnchor, c_bitsForBody, c_bitsForTitle, c_bitsForUrl, c_bitsForPosition,  c_bitsForShard> TermFeatures;
            typedef Model<TermFeatures> TermModel;

            typedef Packed<2> ClickFeature;
            typedef Model<ClickFeature> ClickModel;

            typedef Packed<1> BoolFeature;
            typedef Model<BoolFeature> LanguageModel;
            typedef Model<BoolFeature> LocationModel;

            typedef uint64_t TermHash;
            typedef uint64_t DocId;
            typedef uint32_t Shard;

            typedef uint32_t DocIndex;

            struct MarketData
            {
                uint32_t m_languageHash;
                uint32_t m_locationHash;
            };


            struct QueryContext
            {
                MarketData m_queryMarket;
            };


            typedef bool (*HashLookupFunc)(void const * buffer, unsigned slotCount, uint64_t key, uint64_t& value);

#pragma pack(push, 1)
            struct WebDocData
            {
                float m_staticScore;
                float m_advancedPreferScore;

                MarketData m_documentMarket;

                // Extracted from a metaword used to store the FILETIME of document's discovery.
                uint64_t m_discoveryTimeTicks;

                unsigned m_termSlotCount;
                unsigned m_clickSlotCount;
            };
#pragma pack(pop)

            typedef unsigned FixedSizeBlobId;
            typedef unsigned VariableSizeBlobId;


            class DocumentHandle : NonCopyable
            {
            public:
                DocumentHandle(std::map<FixedSizeBlobId, void*> const & fixedSizeBlobs,
                               std::map<VariableSizeBlobId, void*> const & variableSizeBlobs)
                    : m_fixedSizeBlobs(fixedSizeBlobs),
                      m_variableSizeBlobs(variableSizeBlobs)
                {
                }


                // Note: at least in this test, the blobs are accessed through
                // a function call instead of JITGetFixedSizeBlob() method
                // creating a Node to do so.
                void* GetFixedSizeBlob(FixedSizeBlobId blobId) const
                {
                    auto blobIt = m_fixedSizeBlobs.find(blobId);

                    LogThrowAssert(blobIt != m_fixedSizeBlobs.end(),
                                   "Unknown fixed size blob ID %u",
                                   blobId);

                    return blobIt->second;
                }


                void* GetVariableSizeBlob(VariableSizeBlobId blobId) const
                {
                    auto blobIt = m_variableSizeBlobs.find(blobId);

                    LogThrowAssert(blobIt != m_variableSizeBlobs.end(),
                                   "Unknown variable size blob ID %u",
                                   blobId);

                    return blobIt->second;
                }

            private:
                std::map<FixedSizeBlobId, void*> const & m_fixedSizeBlobs;
                std::map<VariableSizeBlobId, void*> const & m_variableSizeBlobs;
            };


            struct WebModelSetV1
            {
                LanguageModel* m_languageModel;
                LocationModel* m_locationModel;
                TermModel* m_termModel;
                ClickModel* m_clickModel;
            };


            // The CommonRankerContext tries to simulate a scenario where
            // the code is being compiled on MLA and executed on OS, so it
            // tries to keep the size of the context fixed for compatibility.
            struct CommonRankerContext
            {
                void* (*m_getFixedSizeBlobFunc)(DocumentHandle const * handle,
                                                FixedSizeBlobId id);
                void* (*m_getVariableSizeBlobFunc)(DocumentHandle const * handle,
                                                   VariableSizeBlobId id);

                void* m_modelSet;

                char m_fixedSizePadding[64 - 3 * sizeof(void*)];
            };

            static_assert(std::is_pod<CommonRankerContext>::value,
                          "CommonRankerContext must be a POD");
            static_assert(sizeof(CommonRankerContext) == 64,
                          "CommonRankerContext must have a fixed size");


            struct WebRankerContext
            {
                struct CommonRankerContext m_commonContext;

                FixedSizeBlobId m_docDataBlobId;

                VariableSizeBlobId m_termFreqTableBlobId;
                VariableSizeBlobId m_clickFreqTableBlobId;

                HashLookupFunc m_termLookupFunc;
                HashLookupFunc m_clickLookupFunc;
            };

            static_assert(std::is_pod<WebRankerContext>::value,
                          "WebRankerContext must be a POD");

            static const TermFrequencies c_defaultTermFrequencies;


            // Creates term features out of frequencies and position and shard.
            static TermFeatures MakeTermFeatures(TermFrequencies const & frequencies,
                                                 uint8_t position,
                                                 Shard shard)
            {
                return frequencies
                    .InsertRightmost<c_bitsForPosition>(position)
                    .InsertRightmost<c_bitsForShard>(shard);
            }


            // Some test constants.
            static const TermHash c_redHash;

            static const TermHash c_dogHash;
            static const TermHash c_dogsHash;

            static const TermHash c_houseHash;
            static const TermHash c_housesHash;

            static const float c_anyIdf1;
            static const float c_anyIdf2;
            static const float c_anyIdf3;
            static const float c_anyIdf4;

            // For click phrase bigram.
            static const TermHash c_dogHouseHash = 0x5236264;
            static const TermHash c_nonExistentPhraseHash = 0x3475897234;

            static const unsigned c_expectedSlotCount = 0xC0DE;

            // A single term anywhere inside the query (on its own, as a part of
            // phrase, inside word: operator).
            struct TermInfo
            {
                TermInfo(TermHash hash, float idf)
                    : m_hash(hash),
                      m_idf(idf)
                {
                }

                TermHash m_hash;
                float m_idf;
            };


            // Unigram ("dog"), bigram ("dog house") etc. to represent a single
            // word or a multi-word phrase.
            struct QueryNGram
            {
                QueryNGram(std::initializer_list<TermInfo> list)
                    : m_terms(list)
                {
                }

                std::vector<TermInfo> m_terms;
            };


            // A query component at a certain position. A single n-gram represents
            // a single word or a phrase, whereas multiple n-grams represent
            // various candidates. For example, word:(ny "new york") is a query
            // component with two n-grams: a unigram "ny" and a bigram "new york".
            struct QueryComponent
            {
                QueryComponent(std::initializer_list<QueryNGram> list)
                    : m_candidates(list)
                {
                }

                std::vector<QueryNGram> m_candidates;
            };


            // All the word components that make up a query from the scoring
            // perspective.
            struct QueryWords
            {
                QueryWords(std::initializer_list<QueryComponent> list)
                    : m_components(list)
                {
                }

                std::vector<QueryComponent> m_components;
            };


            // A simple class to handle values that may or may not be provided.
            template <typename T>
            class Nullable
            {
            public:
                Nullable()
                    : m_hasValue(false),
                      m_value()
                {
                }


                Nullable(T const & value)
                    : m_hasValue(true),
                      m_value(value)
                {
                }


                void Set(T const & value)
                {
                    m_value = value;
                    m_hasValue = true;
                }


                bool HasValue() const
                {
                    return m_hasValue;
                }


                T& Get()
                {
                    LogThrowAssert(m_hasValue, "Cannot Get() null value");
                    return m_value;
                }

                T const & Get() const
                {
                    LogThrowAssert(m_hasValue, "Cannot Get() null value");
                    return m_value;
                }

            private:
                bool m_hasValue;
                T m_value;
            };


            // A constant used in rangeconstraint: behavior simulation.
            static const float c_discardedDocumentScore;

            static const MarketData c_anyMarketData;
            static const MarketData c_anyOtherMarketDataWithSameLanguage;
            static const MarketData c_anyOtherFullyDifferentMarketData;
            static const MarketData c_clickPhraseMarketData;

            struct ParsedQuery
            {
                ParsedQuery(QueryWords const & queryWords, MarketData const & queryMarket)
                    : m_queryWords(queryWords),
                      m_queryMarket(queryMarket)
                {
                }


                ParsedQuery& SetClickPhrase(TermHash clickPhrase)
                {
                    m_clickPhrase.Set(clickPhrase);

                    return *this;
                }


                ParsedQuery& RequireDiscoveryTimeTicksLessThan(uint64_t value)
                {
                    m_discoveryTimeTicksLessThanConstraint.Set(value);

                    return *this;
                }


                QueryWords m_queryWords;
                MarketData m_queryMarket;

                Nullable<TermHash> m_clickPhrase;

                // Used in rangeconstraint operator simulation: if non-null then
                // DocData::m_discoveryTimeTicks must match the constraint (its value
                // must be lower than the value inside the nullable).
                Nullable<uint64_t> m_discoveryTimeTicksLessThanConstraint;
            };


            // A class used to evaluate query words through the term model using
            // the C++ constructs, to be compared against the NativeJIT version.
            class TermEvaluationContextCPlusPlus : private NonCopyable
            {
            public:
                TermEvaluationContextCPlusPlus(
                    WebRankerContext const * rankerContext,
                    TermModel* termModel,
                    WebDocData const * docData,
                    void const * termFreqTable,
                    Shard shard)
                    : m_termFreqTable(termFreqTable),
                      m_termSlotCount(docData->m_termSlotCount),
                      m_termLookupFunc(rankerContext->m_termLookupFunc),
                      m_termModel(termModel),
                      m_shard(shard)
                {
                }


                float CalculateQueryWordsScore(QueryWords const & queryWords)
                {
                    auto const & components = queryWords.m_components;
                    float score = 0;

                    for(uint8_t position = 0;
                        position < components.size();
                        ++position)
                    {
                        float idf;
                        TermFrequencies frequencies
                            = EstimateQueryComponentFrequencyAndIDF(components[position],
                                                                    idf);

                        TermFeatures termFeatures = MakeTermFeatures(frequencies, position, m_shard);
                        auto termModelScore = m_termModel->Apply(termFeatures);

                        score += termModelScore * idf;
                    }

                    return score;
                }


            private:
                // Per-component aggregation of two TermFrequencies using the
                // specified function.
                TermFrequencies Aggregate(TermFrequencies f1ABTU,
                                          TermFrequencies f2ABTU,
                                          PackedUnderlyingType (*aggFunc)(PackedUnderlyingType,
                                                                          PackedUnderlyingType))
                {
                    auto anchor = aggFunc(f1ABTU.Leftmost(), f2ABTU.Leftmost());
                    auto f1BTU = f1ABTU.WithoutLeftmost();
                    auto f2BTU = f2ABTU.WithoutLeftmost();

                    auto body = aggFunc(f1BTU.Leftmost(), f2BTU.Leftmost());
                    auto f1TU = f1BTU.WithoutLeftmost();
                    auto f2TU = f2BTU.WithoutLeftmost();

                    auto title = aggFunc(f1TU.Leftmost(), f2TU.Leftmost());
                    auto f1U = f1TU.WithoutLeftmost();
                    auto f2U = f2TU.WithoutLeftmost();

                    auto url = aggFunc(f1U.Leftmost(), f2U.Leftmost());

                    return TermFrequencies::FromComponents(anchor, body, title, url);
                }


                TermFrequencies PackedMin(TermFrequencies f1ABTU, TermFrequencies f2ABTU)
                {
                    return Aggregate(f1ABTU,
                                     f2ABTU,
                                     [](PackedUnderlyingType a, PackedUnderlyingType b)
                                     {
                                         return (std::min)(a, b);
                                     });
                }


                TermFrequencies PackedMax(TermFrequencies f1ABTU, TermFrequencies f2ABTU)
                {
                    return Aggregate(f1ABTU,
                                     f2ABTU,
                                     [](PackedUnderlyingType a, PackedUnderlyingType b)
                                     {
                                         return (std::max)(a, b);
                                     });
                }


                TermFrequencies LookupTermFrequencies(TermHash termHash)
                {
                    uint64_t termFrequenciesRaw;
                    const bool lookupSuccessful = m_termLookupFunc(m_termFreqTable,
                                                                   m_termSlotCount,
                                                                   termHash,
                                                                   termFrequenciesRaw);

                    if (!lookupSuccessful)
                    {
                        termFrequenciesRaw = c_defaultTermFrequencies;
                    }


                    return TermFrequencies::FromBits(static_cast<PackedUnderlyingType>(termFrequenciesRaw));
                }


                TermFrequencies
                EstimateNGramFrequencyAndIDF(QueryNGram const & nGram,
                                             float & estimatedIdf)
                {
                    auto const & termInfos = nGram.m_terms;

                    LogThrowAssert(termInfos.size() > 0, "Cannot process n-gram with zero words");

                    TermFrequencies currFrequencies = LookupTermFrequencies(termInfos[0].m_hash);
                    float currentIdf = termInfos[0].m_idf;

                    // See the Node version of this method for details about the
                    // logic.
                    for (unsigned i = 1; i < termInfos.size(); ++i)
                    {
                        TermFrequencies frequencies = LookupTermFrequencies(termInfos[i].m_hash);

                        currFrequencies = PackedMin(currFrequencies, frequencies);
                        currentIdf = (std::max)(currentIdf, termInfos[i].m_idf);
                    }

                    estimatedIdf = currentIdf;
                    return currFrequencies;
                }


                TermFrequencies
                EstimateQueryComponentFrequencyAndIDF(QueryComponent const & component,
                                                      float & estimatedIdf)
                {
                    auto const & candidates = component.m_candidates;
                    LogThrowAssert(candidates.size() > 0, "Cannot process an empty query component");

                    float currentIdf;
                    TermFrequencies currFrequencies
                        = EstimateNGramFrequencyAndIDF(candidates[0], currentIdf);

                    // See the Node version of this method for details about the
                    // logic.
                    for (unsigned i = 1; i < candidates.size(); ++i)
                    {
                        // Get the data for the current candidate.
                        float idf;
                        TermFrequencies frequencies
                            = EstimateNGramFrequencyAndIDF(candidates[i], idf);

                        currFrequencies = PackedMax(currFrequencies, frequencies);
                        currentIdf = (std::min)(currentIdf, idf);
                    }

                    estimatedIdf = currentIdf;
                    return currFrequencies;
                }


                void const * m_termFreqTable;
                unsigned m_termSlotCount;

                HashLookupFunc m_termLookupFunc;
                TermModel const * m_termModel;

                Shard m_shard;
            };


            static uint16_t ComputeClickHash(uint32_t queryLanguageHash,
                                                     uint32_t queryLocationHash,
                                                     TermHash clickPhrase)
            {
                const uint64_t lang64 = queryLanguageHash;
                const uint64_t loc64 = queryLocationHash;

                // (language64 | (location64 << 32)) + clickPhrase)
                const uint64_t clickHash64
                    = (lang64 | (loc64 << 32)) + clickPhrase;

                // Keep only the lowest 16 bits.
                // Note: the original calculation uses % 65521 (the largest
                // prime 16-bit number) to calculate the value.
                return static_cast<uint16_t>(clickHash64);
            }


            static float GetClickPhraseScore(WebRankerContext const * rankerContext,
                                             ClickModel* clickModel,
                                             WebDocData const * docData,
                                             void const * clickFreqTable,
                                             uint32_t queryLanguageHash,
                                             uint32_t queryLocationHash,
                                             TermHash clickPhrase)
            {
                auto const clickHash = ComputeClickHash(queryLanguageHash,
                                                        queryLocationHash,
                                                        clickPhrase);
                uint64_t clickFeatureRaw;

                const bool lookupSuccessful
                    = rankerContext->m_clickLookupFunc(clickFreqTable,
                                                       docData->m_clickSlotCount,
                                                       clickHash,
                                                       clickFeatureRaw);

                if (!lookupSuccessful)
                {
                    clickFeatureRaw = 0;
                }

                auto clickFeature = ClickFeature::FromBits(static_cast<PackedUnderlyingType>(clickFeatureRaw));

                return clickModel->Apply(clickFeature);
            }


            static float CalculateScore(ParsedQuery const & parsedQuery,
                                        Shard shard,
                                        DocumentHandle const * docHandle,
                                        void const * rankerContextRaw,
                                        QueryContext const * queryContext)
            {
                auto const rankerContext = static_cast<WebRankerContext const *>(rankerContextRaw);
                auto & commonRankerContext = rankerContext->m_commonContext;

                // Get WebDocData with static score and advance prefer score.
                auto getFixedSizedBlobFunc = commonRankerContext.m_getFixedSizeBlobFunc;

                auto docData = reinterpret_cast<WebDocData const *>(
                    getFixedSizedBlobFunc(docHandle, rankerContext->m_docDataBlobId));

                // Check if the document should be discarded.
                if (parsedQuery.m_discoveryTimeTicksLessThanConstraint.HasValue()
                    && !(docData->m_discoveryTimeTicks
                         < parsedQuery.m_discoveryTimeTicksLessThanConstraint.Get()))
                {
                    return c_discardedDocumentScore;
                }

                auto const * modelSet = reinterpret_cast<WebModelSetV1 const *>(commonRankerContext.m_modelSet);

                // Initialize the score to the static score.
                float totalScore = docData->m_staticScore;

                // Get query and document language and compare them.
                auto docLanguage = docData->m_documentMarket.m_languageHash;
                auto queryLanguage = queryContext->m_queryMarket.m_languageHash;

                const unsigned languageMatches = docLanguage == queryLanguage ? 1 : 0;
                totalScore += modelSet->m_languageModel->Apply(BoolFeature::FromBits(languageMatches));

                // Do the same for the location.
                auto docLocation = docData->m_documentMarket.m_locationHash;
                auto queryLocation = queryContext->m_queryMarket.m_locationHash;

                const unsigned locationMatches = docLocation == queryLocation ? 1: 0;
                totalScore += modelSet->m_locationModel->Apply(BoolFeature::FromBits(locationMatches));

                // If both language and location match, use the advanced prefer score.
                if (languageMatches && locationMatches)
                {
                    totalScore += docData->m_advancedPreferScore;
                }

                auto getVarSizedBlobFunc = commonRankerContext.m_getVariableSizeBlobFunc;
                auto & queryWords = parsedQuery.m_queryWords;

                // Prepare to evaluate term and click models.
                if (!queryWords.m_components.empty())
                {
                    auto termHashTable = getVarSizedBlobFunc(docHandle, rankerContext->m_termFreqTableBlobId);
                    auto termModel = modelSet->m_termModel;

                    // Evaluate the term model.
                    TermEvaluationContextCPlusPlus termContext(rankerContext,
                                                               termModel,
                                                               docData,
                                                               termHashTable,
                                                               shard);

                    totalScore += termContext.CalculateQueryWordsScore(queryWords);
                }

                if (parsedQuery.m_clickPhrase.HasValue())
                {
                    auto clickHashTable = getVarSizedBlobFunc(docHandle, rankerContext->m_clickFreqTableBlobId);
                    auto clickModel = modelSet->m_clickModel;

                    totalScore += GetClickPhraseScore(rankerContext,
                                                      clickModel,
                                                      docData,
                                                      clickHashTable,
                                                      queryLanguage,
                                                      queryLocation,
                                                      parsedQuery.m_clickPhrase.Get());
                }

                return totalScore;
            }


            // A class used to evaluate query words through the term model using
            // the NativeJIT constructs, to be compared against the C++ version.
            class TermEvaluationContextNativeJIT : private NonCopyable
            {
            public:
                TermEvaluationContextNativeJIT(ExpressionNodeFactory& e,
                                               Node<WebRankerContext const *>& rankerContext,
                                               Node<TermModel*>& termModel,
                                               Node<WebDocData const *>& docData,
                                               Node<void const *>& termFreqTable,
                                               Node<Shard>& shard)
                    : m_termFreqTable(termFreqTable),
                      m_termSlotCount(e.Deref(e.FieldPointer(docData, &WebDocData::m_termSlotCount))),
                      m_termLookupFunc(e.Deref(e.FieldPointer(rankerContext, &WebRankerContext::m_termLookupFunc))),
                      m_defaultTermFrequencies(e.Immediate(c_defaultTermFrequencies)),
                      m_termModel(termModel),
                      m_shardShiftedToMSB(e.Shl(e.Cast<PackedUnderlyingType>(shard),
                                                static_cast<uint8_t>(sizeof(PackedUnderlyingType) * 8 - c_bitsForShard)))
                {
                }


                Node<float>& AddQueryWordsScore(ExpressionNodeFactory& e,
                                                QueryWords const & queryWords,
                                                Node<float>& currentScore)
                {
                    Node<float>* updatedScore = &currentScore;
                    auto & components = queryWords.m_components;

                    for(uint8_t position = 0;
                        position < components.size();
                        ++position)
                    {
                        float idf;
                        auto & frequencies
                            = EstimateQueryComponentFrequencyAndIDF(e,
                                                                    components[position],
                                                                    idf);

                        auto & termFeatures = GetTermFeatures(e,
                                                              frequencies,
                                                              position);
                        auto & termModelScore = e.ApplyModel(m_termModel, termFeatures);
                        auto & componentScore = e.Mul(termModelScore, e.Immediate(idf));

                        updatedScore = &e.Add(*updatedScore, componentScore);
                    }

                    return *updatedScore;
                }


            private:
                // Given a term hash, lookup the corresponding term frequencies
                // from the hash table.
                Node<TermFrequencies>& LookupTermFrequencies(ExpressionNodeFactory& e,
                                                             TermHash term)
                {
                    auto & termFrequenciesRaw = e.StackVariable<uint64_t>();
                    auto & termHash = e.Immediate(term);
                    auto & termLookupSuccessful = e.Call(m_termLookupFunc,
                                                         m_termFreqTable,
                                                         m_termSlotCount,
                                                         termHash,
                                                         termFrequenciesRaw);

                    auto & dereferencedRawFrequencies
                        = e.Dependent(e.Deref(termFrequenciesRaw),
                                      termLookupSuccessful);

                    auto & termFrequencies = e.If(termLookupSuccessful,
                                                  e.Cast<TermFrequencies>(dereferencedRawFrequencies),
                                                  m_defaultTermFrequencies);

                    return termFrequencies;
                }


                Node<TermFeatures>& GetTermFeatures(ExpressionNodeFactory& e,
                                                    Node<TermFrequencies>& frequencies,
                                                    uint8_t position)
                {
                    const PackedUnderlyingType positionShiftedToMSB
                        = static_cast<PackedUnderlyingType>(position) << (sizeof(PackedUnderlyingType) * 8 - c_bitsForPosition);

                    // Shift the position from the MSB position into the
                    // LSB position in the target variable.
                    auto & frequenciesAndShard
                        = e.Shld(e.Cast<PackedUnderlyingType>(frequencies),
                                 e.Immediate(positionShiftedToMSB),
                                 c_bitsForPosition);

                    // Do the same for shard to finalize the full term features.
                    auto & rawTermFeatures
                        = e.Shld(frequenciesAndShard,
                                 m_shardShiftedToMSB,
                                 c_bitsForShard);

                    return e.Cast<TermFeatures>(rawTermFeatures);
                }


                Node<TermFrequencies>&
                EstimateNGramFrequencyAndIDF(ExpressionNodeFactory& e,
                                             QueryNGram const & nGram,
                                             float & estimatedIdf)
                {
                    auto & termInfos = nGram.m_terms;

                    LogThrowAssert(termInfos.size() > 0, "Cannot process n-gram with zero words");

                    Node<TermFrequencies>* currFrequencies
                        = &LookupTermFrequencies(e, termInfos[0].m_hash);
                    float currentIdf = termInfos[0].m_idf;

                    for (unsigned i = 1; i < termInfos.size(); ++i)
                    {
                        // The "word1 word2" phrase can appear at most min(word1AppearanceCount,
                        // word2AppearanceCount) times in the document.
                        auto & frequencies = LookupTermFrequencies(e, termInfos[i].m_hash);
                        currFrequencies = &e.PackedMin(*currFrequencies, frequencies);

                        // IDF is a measure of term's value with regard to its unambiguity - if
                        // a term is rare (and thus its IDF is high) then the search is more
                        // specific and the term adds more value to it.
                        // A phrase search is more specific and thus has more value than each
                        // of the terms on their own. That value is approximated by taking a
                        // maximum of the two IDFs.
                        currentIdf = (std::max)(currentIdf, termInfos[i].m_idf);
                    }

                    estimatedIdf = currentIdf;
                    return *currFrequencies;
                }


                Node<TermFrequencies>&
                EstimateQueryComponentFrequencyAndIDF(ExpressionNodeFactory& e,
                                                      QueryComponent const & component,
                                                      float & estimatedIdf)
                {
                    auto & candidates = component.m_candidates;
                    LogThrowAssert(candidates.size() > 0, "Cannot process an empty query component");

                    float currentIdf;
                    Node<TermFrequencies>* currFrequencies
                        = &EstimateNGramFrequencyAndIDF(e,
                                                        candidates[0],
                                                        currentIdf);

                    for (unsigned i = 1; i < candidates.size(); ++i)
                    {
                        // Get the data for the current candidate.
                        float idf;
                        auto & frequencies = EstimateNGramFrequencyAndIDF(e,
                                                                          candidates[i],
                                                                          idf);

                        // Use a maximum of the two term's frequencies as term frequency for
                        // word candidates. Note: the sum of the frequencies may seem like
                        // a better fit but it is a bit more complicated to calculate when term
                        // frequencies are log conditioned and also experiments show almost
                        // nonexistent relevance difference when it is used.
                        currFrequencies = &e.PackedMax(*currFrequencies, frequencies);

                        // IDF is a measure of term's value with regard to its unambiguity - if
                        // a term is rare (and thus its IDF is high) then the search is more
                        // specific and the term adds more value to it.
                        // A search with word candidates is less specific and thus has less
                        // value than each of the terms on their own. That value is approximated
                        // by taking a minimum of the two IDFs.
                        currentIdf = (std::min)(currentIdf, idf);
                    }

                    estimatedIdf = currentIdf;
                    return *currFrequencies;
                }


                Node<void const *>& m_termFreqTable;
                Node<unsigned>& m_termSlotCount;

                Node<HashLookupFunc>& m_termLookupFunc;
                Node<TermFrequencies>& m_defaultTermFrequencies;

                Node<TermModel*>& m_termModel;

                Node<PackedUnderlyingType>& m_shardShiftedToMSB;
            };


            Node<uint16_t>& ComputeClickHash(
                ExpressionNodeFactory& e,
                Node<uint32_t>& queryLanguageHash,
                Node<uint32_t>& queryLocationHash,
                TermHash clickPhrase)
            {
                auto & lang64 = e.Cast<uint64_t>(queryLanguageHash);
                auto & loc64 = e.Cast<uint64_t>(queryLocationHash);

                // (language64 | (location64 << 32)) + clickPhrase)
                auto & clickHash64 = e.Add(e.Or(lang64,
                                                e.Shl(loc64, static_cast<uint8_t>(32))),
                                           e.Immediate(clickPhrase));

                // Keep only the lowest 16 bits.
                // Note: the original calculation uses % 65521 (the largest
                // prime 16-bit number) to calculate the value.
                return e.Cast<uint16_t>(clickHash64);
            }


            Node<float>&
            GetClickPhraseScore(ExpressionNodeFactory& e,
                                Node<WebRankerContext const *>& rankerContext,
                                Node<ClickModel*>& clickModel,
                                Node<WebDocData const *>& docData,
                                Node<void const *>& clickFreqTable,
                                Node<uint32_t>& queryLanguageHash,
                                Node<uint32_t>& queryLocationHash,
                                TermHash clickPhrase)
            {
                auto & clickHash = ComputeClickHash(e, queryLanguageHash, queryLocationHash, clickPhrase);
                auto & clickFeatureRaw = e.StackVariable<uint64_t>();

                auto & clickLookupSuccessful = e.Call(e.Deref(e.FieldPointer(rankerContext, &WebRankerContext::m_clickLookupFunc)),
                                                      clickFreqTable,
                                                      e.Deref(e.FieldPointer(docData, &WebDocData::m_clickSlotCount)),
                                                      e.Cast<uint64_t>(clickHash),
                                                      clickFeatureRaw);

                auto & dereferencedRawFeature
                    = e.Dependent(e.Deref(clickFeatureRaw),
                                  clickLookupSuccessful);

                auto & clickFeature = e.If(clickLookupSuccessful,
                                           e.Cast<ClickFeature>(dereferencedRawFeature),
                                           e.Immediate(ClickFeature::FromBits(0)));

                return e.ApplyModel(clickModel, clickFeature);
            }


            typedef float (*ScoringFunction)(Shard, DocumentHandle const *, void const *, QueryContext const *);

            ScoringFunction
            BuildAndCompileScoringFunction(ParsedQuery const & parsedQuery,
                                           TestCaseSetup& setup)
            {
                Function<float, Shard, DocumentHandle const *, void const *, QueryContext const *>
                    e(setup.GetAllocator(), setup.GetCode());

                auto & shard = e.GetP1();
                auto & docHandle = e.GetP2();
                auto & rankerContext = e.Cast<WebRankerContext const *>(e.GetP3());
                auto & queryContext = e.GetP4();

                auto & commonRankerContext = e.FieldPointer(rankerContext, &WebRankerContext::m_commonContext);
                auto & modelSet = e.Cast<WebModelSetV1 const *>(e.Deref(e.FieldPointer(commonRankerContext, &CommonRankerContext::m_modelSet)));
                auto & floatZero = e.Immediate<float>(0);

                // Get WebDocData with static score and advance prefer score.
                auto & getFixedSizedBlobFunc = e.Deref(e.FieldPointer(commonRankerContext, &CommonRankerContext::m_getFixedSizeBlobFunc));

                auto & docDataBlobId = e.Deref(e.FieldPointer(rankerContext, &WebRankerContext::m_docDataBlobId));
                auto & docData = e.Cast<WebDocData const *>(e.Call(getFixedSizedBlobFunc, docHandle, docDataBlobId));

                // Initialize the score to the static score.
                auto & staticScore = e.Deref(e.FieldPointer(docData, &WebDocData::m_staticScore));
                auto totalScore = &staticScore;

                // Get query and document market.
                auto & docMarket = e.FieldPointer(docData, &WebDocData::m_documentMarket);
                auto & queryMarket = e.FieldPointer(queryContext, &QueryContext::m_queryMarket);

                // Get query and document language and compare them.
                auto & docLanguage = e.Deref(e.FieldPointer(docMarket, &MarketData::m_languageHash));
                auto & queryLanguage = e.Deref(e.FieldPointer(queryMarket, &MarketData::m_languageHash));

                auto & languageMatches = e.Compare<JccType::JE>(docLanguage, queryLanguage);
                auto & languageModel = e.Deref(e.FieldPointer(modelSet, &WebModelSetV1::m_languageModel));

                // Evaluate the language model.
                totalScore = &e.Add(*totalScore,
                                    e.ApplyModel(languageModel,
                                                 e.Cast<BoolFeature>(e.Cast<PackedUnderlyingType>(languageMatches))));

                // Do the same for the location.
                auto & docLocation = e.Deref(e.FieldPointer(docMarket, &MarketData::m_locationHash));
                auto & queryLocation = e.Deref(e.FieldPointer(queryMarket, &MarketData::m_locationHash));

                auto & locationMatches = e.Compare<JccType::JE>(docLocation, queryLocation);
                auto & locationModel = e.Deref(e.FieldPointer(modelSet, &WebModelSetV1::m_locationModel));

                totalScore = &e.Add(*totalScore,
                                    e.ApplyModel(locationModel,
                                                 e.Cast<BoolFeature>(e.Cast<PackedUnderlyingType>(locationMatches))));

                // If both language and location match, add the advanced prefer score.
                auto & marketMatches = e.And(languageMatches, locationMatches);
                auto & advPreferScore = e.Deref(e.FieldPointer(docData, &WebDocData::m_advancedPreferScore));

                totalScore = &e.Add(*totalScore,
                                    e.If(marketMatches, advPreferScore, floatZero));

                auto & queryWords = parsedQuery.m_queryWords;

                // Prepare to evaluate term and click models.
                if (!queryWords.m_components.empty()
                    || parsedQuery.m_clickPhrase.HasValue())
                {
                    auto & getVarSizedBlobFunc = e.Deref(e.FieldPointer(commonRankerContext, &CommonRankerContext::m_getVariableSizeBlobFunc));

                    if (!queryWords.m_components.empty())
                    {
                        auto & termFreqTableBlobId = e.Deref(e.FieldPointer(rankerContext, &WebRankerContext::m_termFreqTableBlobId));
                        auto & termHashTable = e.Call(getVarSizedBlobFunc, docHandle, termFreqTableBlobId);
                        auto & termModel = e.Deref(e.FieldPointer(modelSet, &WebModelSetV1::m_termModel));

                        // Evaluate the term model.
                        TermEvaluationContextNativeJIT termContext(e,
                                                                   rankerContext,
                                                                   termModel,
                                                                   docData,
                                                                   e.AddTargetConstCast(termHashTable),
                                                                   shard);

                        totalScore = &termContext.AddQueryWordsScore(e,
                                                                     queryWords,
                                                                     *totalScore);
                    }

                    if (parsedQuery.m_clickPhrase.HasValue())
                    {
                        auto & clickFreqTableBlobId = e.Deref(e.FieldPointer(rankerContext, &WebRankerContext::m_clickFreqTableBlobId));
                        auto & clickHashTable = e.Call(getVarSizedBlobFunc, docHandle, clickFreqTableBlobId);

                        auto & clickModel = e.Deref(e.FieldPointer(modelSet, &WebModelSetV1::m_clickModel));

                        totalScore = &e.Add(*totalScore,
                                            GetClickPhraseScore(e,
                                                                rankerContext,
                                                                clickModel,
                                                                docData,
                                                                e.AddTargetConstCast(clickHashTable),
                                                                queryLanguage,
                                                                queryLocation,
                                                                parsedQuery.m_clickPhrase.Get()));
                    }
                }

                // If there's a discovery ticks constraint, the expression should
                // only be evaluated if discovery ticks are below the constraint,
                // otherwise a constant value will be returned.
                if (parsedQuery.m_discoveryTimeTicksLessThanConstraint.HasValue())
                {
                    auto & discoveryTicks = e.Deref(e.FieldPointer(docData, &WebDocData::m_discoveryTimeTicks));

                    e.AddExecuteOnlyIfStatement(e.Compare<JccType::JB>(discoveryTicks,
                                                                       e.Immediate(parsedQuery.m_discoveryTimeTicksLessThanConstraint.Get())),
                                                e.Immediate(c_discardedDocumentScore));
                }

                return e.Compile(*totalScore);
            }


            struct DocumentDescriptor : NonCopyable
            {
                DocumentDescriptor(MarketData const & docMarket)
                    : m_documentMarket(docMarket),
                      m_discoveryTimeTicks(0)
                {
                }

                DocumentDescriptor& SetDiscoveryTimeTicks(uint64_t value)
                {
                    m_discoveryTimeTicks = value;

                    return *this;
                }

                const MarketData m_documentMarket;
                uint64_t m_discoveryTimeTicks;
            };


            struct TestData
            {
                TestData(DocumentDescriptor const & docDescriptor,
                         ParsedQuery const & parsedQuery)
                    : m_shard(3),
                      m_docHandle(m_fixedSizeBlobs, m_variableSizeBlobs)
                {
                    m_termFreqMap[c_redHash] = TermFrequencies::FromComponents(4, 3, 1, 1);
                    // c_dogHash not placed into the map to signify a trivial term.
                    m_termFreqMap[c_dogsHash] = TermFrequencies::FromComponents(2, 3, 0, 1);
                    m_termFreqMap[c_houseHash] = TermFrequencies::FromComponents(0, 2, 0, 0);
                    m_termFreqMap[c_housesHash] = TermFrequencies::FromComponents(1, 2, 1, 0);

                    auto const clickHash
                        = ComputeClickHash(c_clickPhraseMarketData.m_languageHash,
                                           c_clickPhraseMarketData.m_locationHash,
                                           c_dogHouseHash);
                    m_clickFreqMap[clickHash] = ClickFeature::FromBits(2);

                    m_docData.m_staticScore = 5.7f;
                    m_docData.m_advancedPreferScore = 1.3f;
                    m_docData.m_documentMarket = docDescriptor.m_documentMarket;
                    m_docData.m_discoveryTimeTicks = docDescriptor.m_discoveryTimeTicks;
                    m_docData.m_termSlotCount = c_expectedSlotCount;
                    m_docData.m_clickSlotCount = c_expectedSlotCount;

                    m_modelSet.m_clickModel = &m_clickModel;
                    m_modelSet.m_languageModel = &m_languageModel;
                    m_modelSet.m_locationModel = &m_locationModel;
                    m_modelSet.m_termModel = &m_termModel;

                    m_fixedSizeBlobs[c_docDataBlobId] = &m_docData;
                    m_variableSizeBlobs[c_termFreqTableBlobId] = &m_termFreqMap;
                    m_variableSizeBlobs[c_clickFreqTableBlobId] = &m_clickFreqMap;

                    m_rankerContext.m_commonContext.m_getFixedSizeBlobFunc = &GetFixedSizeBlobFunc;
                    m_rankerContext.m_commonContext.m_getVariableSizeBlobFunc = &GetVariableSizeBlobFunc;
                    m_rankerContext.m_commonContext.m_modelSet = &m_modelSet;

                    m_rankerContext.m_docDataBlobId = c_docDataBlobId;
                    m_rankerContext.m_termFreqTableBlobId = c_termFreqTableBlobId;
                    m_rankerContext.m_clickFreqTableBlobId = c_clickFreqTableBlobId;

                    m_rankerContext.m_termLookupFunc = &HashLookupFunc<TermFrequencies>;
                    m_rankerContext.m_clickLookupFunc = &HashLookupFunc<ClickFeature>;

                    m_queryContext.m_queryMarket = parsedQuery.m_queryMarket;

                    InitModel(m_languageModel, 0.37f, 0.72f);
                    InitModel(m_locationModel, 0.12f, 0.05f);
                    InitModel(m_clickModel, 1.23f, 0.26f);
                    InitModel(m_termModel, 1, 0.01f);
                }

                const Shard m_shard;

                DocumentHandle m_docHandle;
                WebRankerContext m_rankerContext;
                QueryContext m_queryContext;

            private:
                FixedSizeBlobId c_docDataBlobId = 2;
                VariableSizeBlobId c_termFreqTableBlobId = 5;
                VariableSizeBlobId c_clickFreqTableBlobId = 6;

                WebDocData m_docData;

                LanguageModel m_languageModel;
                LocationModel m_locationModel;
                TermModel m_termModel;
                ClickModel m_clickModel;

                WebModelSetV1 m_modelSet;

                std::map<FixedSizeBlobId, void*> m_fixedSizeBlobs;
                std::map<VariableSizeBlobId, void*> m_variableSizeBlobs;

                std::map<uint64_t, TermFrequencies> m_termFreqMap;
                std::map<uint64_t, ClickFeature> m_clickFreqMap;


                static void* GetFixedSizeBlobFunc(DocumentHandle const * handle,
                                                  FixedSizeBlobId id)
                {
                    return handle->GetFixedSizeBlob(id);
                }


                static void* GetVariableSizeBlobFunc(DocumentHandle const * handle,
                                                     VariableSizeBlobId id)
                {
                    return handle->GetVariableSizeBlob(id);
                }


                template <typename VALUE>
                static bool HashLookupFunc(void const * buffer,
                                           unsigned slotCount,
                                           uint64_t key,
                                           uint64_t& value)
                {
                    LogThrowAssert(c_expectedSlotCount == slotCount,
                           "Mismatched slot count, %u vs %u",
                           c_expectedSlotCount,
                           slotCount);

                    auto map = reinterpret_cast<std::map<uint64_t, VALUE> const *>(buffer);

                    auto it = map->find(key);
                    bool found = it != map->end();

                    if (found)
                    {
                        value = it->second;
                    }

                    return found;
                }


                template <typename T>
                static void InitModel(T& model, float startValue, float stepValue)
                {
                    float currValue = startValue;

                    for (unsigned i = 0; i < T::c_size; ++i)
                    {
                        model[i] = currValue;
                        currValue += stepValue;
                    }
                }
            };


            void RunTestCase(DocumentDescriptor const & docDescriptor,
                             ParsedQuery const & parsedQuery)
            {
                auto setup = GetSetup();

                {
                    ASSERT_EQ(0u, offsetof(WebRankerContext, m_commonContext))
                        <<  "Invalid WebRankerContext structure layout";

                    // TestData is large, allocate from heap to avoid stack overflow.
                    auto testData = std::make_unique<TestData>(docDescriptor, parsedQuery);
                    auto expected = CalculateScore(parsedQuery,
                                                   testData->m_shard,
                                                   &testData->m_docHandle,
                                                   &testData->m_rankerContext.m_commonContext,
                                                   &testData->m_queryContext);

                    auto function = BuildAndCompileScoringFunction(parsedQuery, *setup);
                    auto actual = function(testData->m_shard,
                                           &testData->m_docHandle,
                                           &testData->m_rankerContext.m_commonContext,
                                           &testData->m_queryContext);

                    ASSERT_TRUE(std::abs(actual - expected) < 0.0001) <<
                      "Expected score: " << expected << " Actual score: " << actual;
                }
            }


        TEST_FIXTURE_END_TEST_CASES_BEGIN

        TEST_F(Acceptance, SingleWord)
        {
            // house
            QueryWords queryWords
            {
                {
                    QueryComponent
                    {
                        QueryNGram { TermInfo(c_houseHash, c_anyIdf2) }
                    }
                }
            };

            const MarketData market = c_anyMarketData;

            DocumentDescriptor docDescriptor(market);
            ParsedQuery query(queryWords, market);

            RunTestCase(docDescriptor, query);
        }


        TEST_F(Acceptance, MultipleWords)
        {
            // red dog house
            QueryWords queryWords
            {
                {
                    QueryComponent
                    {
                        QueryNGram { TermInfo(c_redHash, c_anyIdf1) }
                    },

                    QueryComponent
                    {
                        QueryNGram { TermInfo(c_dogHash, c_anyIdf2) }
                    },

                    QueryComponent
                    {
                        QueryNGram { TermInfo(c_houseHash, c_anyIdf3) }
                    }
                }
            };

            const MarketData market = c_anyMarketData;

            DocumentDescriptor docDescriptor(market);
            ParsedQuery query(queryWords, market);

            RunTestCase(docDescriptor, query);
        }


        TEST_F(Acceptance, Phrase)
        {
            // red "dog house"
            QueryWords queryWords
            {
                {
                    QueryComponent
                    {
                        QueryNGram { TermInfo(c_redHash, c_anyIdf1) }
                    },

                    QueryComponent
                    {
                        QueryNGram
                        {
                            TermInfo(c_dogHash, c_anyIdf1),
                            TermInfo(c_houseHash, c_anyIdf2)
                        }
                    }
                }
            };

            const MarketData market = c_anyMarketData;

            DocumentDescriptor docDescriptor(market);
            ParsedQuery query(queryWords, market);

            RunTestCase(docDescriptor, query);
        }


        TEST_F(Acceptance, WordCandidates)
        {
            // red word:("dog house" houses)
            QueryWords queryWords
            {
                {
                    QueryComponent
                    {
                        QueryNGram { TermInfo(c_redHash, c_anyIdf4) },
                    },

                    QueryComponent
                    {
                        QueryNGram
                        {
                            TermInfo(c_dogHash, c_anyIdf1),
                            TermInfo(c_houseHash, c_anyIdf3)
                        },
                        QueryNGram { TermInfo(c_housesHash, c_anyIdf4) }
                    }
                }
            };

            const MarketData market = c_anyMarketData;

            DocumentDescriptor docDescriptor(market);
            ParsedQuery query(queryWords, market);

            RunTestCase(docDescriptor, query);
        }


        TEST_F(Acceptance, ClickPhrase)
        {
            QueryWords queryWords
            {
                {
                    QueryComponent
                    {
                        QueryNGram { TermInfo(c_dogHash, c_anyIdf1) },
                        QueryNGram { TermInfo(c_dogsHash, c_anyIdf2) }
                    },

                    QueryComponent
                    {
                        QueryNGram { TermInfo(c_houseHash, c_anyIdf3) },
                    }
                }
            };

            DocumentDescriptor docDescriptor(c_anyMarketData);

            // Click phrase in the click table.
            {
                ParsedQuery query(queryWords, c_clickPhraseMarketData);

                query.SetClickPhrase(c_dogHouseHash);
                RunTestCase(docDescriptor, query);
            }

            // Click phrase not in the click table.
            {
                ParsedQuery query(queryWords, c_clickPhraseMarketData);

                query.SetClickPhrase(c_nonExistentPhraseHash);
                RunTestCase(docDescriptor, query);
            }
        }


        TEST_F(Acceptance, ConstraintDiscard)
        {
            QueryWords queryWords
            {
                {
                    QueryComponent
                    {
                        QueryNGram { TermInfo(c_houseHash, c_anyIdf2) }
                    }
                }
            };

            const MarketData market = c_anyMarketData;

            ParsedQuery query(queryWords, market);
            query.RequireDiscoveryTimeTicksLessThan(10);

            // The following document should not be discarded.
            {
                DocumentDescriptor docDescriptorNoDiscard(market);
                docDescriptorNoDiscard.SetDiscoveryTimeTicks(9);

                RunTestCase(docDescriptorNoDiscard, query);
            }

            // The following document should be discarded.
            {
                DocumentDescriptor docDescriptorDiscard(market);
                docDescriptorDiscard.SetDiscoveryTimeTicks(10);

                RunTestCase(docDescriptorDiscard, query);
            }
        }


        TEST_F(Acceptance, CompletelyDifferentMarket)
        {
            QueryWords queryWords
            {
                {
                    QueryComponent
                    {
                        QueryNGram { TermInfo(c_houseHash, c_anyIdf2) }
                    }
                }
            };

            DocumentDescriptor docDescriptor(c_anyMarketData);
            ParsedQuery query(queryWords, c_anyOtherFullyDifferentMarketData);

            RunTestCase(docDescriptor, query);
        }


        TEST_F(Acceptance, DifferentMarketWithSameLanguage)
        {
            QueryWords queryWords
            {
                {
                    QueryComponent
                    {
                        QueryNGram { TermInfo(c_houseHash, c_anyIdf2) }
                    }
                }
            };

            DocumentDescriptor docDescriptor(c_anyMarketData);
            ParsedQuery query(queryWords, c_anyOtherMarketDataWithSameLanguage);

            RunTestCase(docDescriptor, query);
        }


        TEST_CASES_END

        const Acceptance::TermFrequencies Acceptance::c_defaultTermFrequencies = Acceptance::TermFrequencies::FromComponents(0, 1, 0, 0);

        const float Acceptance::c_discardedDocumentScore = std::numeric_limits<float>::lowest();

        const Acceptance::MarketData Acceptance::c_anyMarketData = {0x01234567, 0x11112222};
        const Acceptance::MarketData Acceptance::c_anyOtherMarketDataWithSameLanguage = {0x01234567, 0x22222222};
        const Acceptance::MarketData Acceptance::c_anyOtherFullyDifferentMarketData = {0x89ABCDEF, 0x33334444};
        const Acceptance::MarketData Acceptance::c_clickPhraseMarketData = {0xC11CC, 0xDA7ADA7A};

        const uint64_t Acceptance::c_redHash = 0x54342434;

        const uint64_t Acceptance::c_dogHash = 0x1233333333;
        const uint64_t Acceptance::c_dogsHash = 0x1255555555;

        const uint64_t Acceptance::c_houseHash = 0x23444444444;
        const uint64_t Acceptance::c_housesHash = 0x2500000001;

        const float Acceptance::c_anyIdf1 = 0.1f;
        const float Acceptance::c_anyIdf2 = 0.23f;
        const float Acceptance::c_anyIdf3 = 0.67f;
        const float Acceptance::c_anyIdf4 = 1.5f;
    }
}
