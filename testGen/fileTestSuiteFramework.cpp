#include <iostream>
#include <memory>
#include <filesystem>
#include <functional>
#include <sstream>

#include <stdio.h>
#include <blast.h>

#include <fileTestSuite/fileTestSuite.h>
#include <fileTestSuite/fileTestSuiteRunner.hpp>
#include <stdint.h>

#include <mio/mmap.hpp>

#include <HydrArgs/HydrArgs.hpp>

#include "SharedLibFactoryHelper.hpp"

pseudospan::pseudospan(uint8_t *first, uint8_t *last):FTSSpanT(first, last){}
pseudospan::pseudospan(uint8_t *buf, size_t len): FTSSpanT(buf, len){}

struct OurProtoTest;

bool pseudospan::check(FTSTestToolkit *tk, const char * ptrDescription, uint8_t *ptr){
	if(ptr < &(*this->begin())){
		tk->processOutOfBound(ptrDescription, "lower", &(*this->begin()));
		return true;
	}
	if(ptr > &(*this->end())){
		tk->processOutOfBound(ptrDescription, "upper", &(*this->end()));
		return true;
	}
	return false;
}

bool pseudospan::check(FTSTestToolkit *tk, const char * ptrDescription, pseudospan &checked){
	std::string ptrDescriptionPostfixed = ptrDescription;
	size_t originalPtrDescrLength = ptrDescriptionPostfixed.size();
	ptrDescriptionPostfixed += " begin";

	if(this->check(tk, ptrDescriptionPostfixed.c_str(), &(*checked.begin()))){
		return true;
	}

	ptrDescriptionPostfixed.resize(originalPtrDescrLength);
	ptrDescriptionPostfixed += " end";

	if(this->check(tk, ptrDescriptionPostfixed.c_str(), &(*checked.end()))){
		return true;
	}
	return false;
}

OutputBufferTester::OutputBufferTester(pseudospan &etalonSpan, FTSTestToolkit *ctx): cumLen(0),etalonSpan(etalonSpan), context(ctx){}

void OutputBufferTester::reset(){
	cumLen = 0;
}

pseudospan OutputBufferTester::getEtalonChunkBoundariesAndUpdateCumLen(size_t len){
	uint8_t *etalonChunkBegin = &(*etalonSpan.begin()) + cumLen;
	cumLen += len;
	uint8_t *etalonChunkEnd = &(*etalonSpan.begin()) + cumLen;
	return pseudospan(etalonChunkBegin, etalonChunkEnd);
}

bool OutputBufferTester::testChunkProcessing(unsigned char *producedChunk, unsigned len){
	auto bufferChunkBounds = pseudospan(producedChunk, len);
	auto etalonChunkBounds = getEtalonChunkBoundariesAndUpdateCumLen(len);
	if(etalonSpan.check(context, "etalon chunk", etalonChunkBounds)){
		return false;
	}

	if(!std::equal(etalonChunkBounds.begin(), etalonChunkBounds.end(), bufferChunkBounds.begin(), bufferChunkBounds.end())){
		context->processInequality(cumLen - len, cumLen);
		return false;
	}

	//auto writtenLen = std::distance(posBefore, posAfter);
	//return writtenLen != len;
	return true;
}


IFileNamePairGen::IFileNamePairGen(std::string const & processedExt):processedExt("." + processedExt){}

bool IFileNamePairGen::isRespFile(std::filesystem::path cand) const{
	return cand.extension() == processedExt;
}

void IFileNamePairGen::processChallengeResponseFilePairs(std::filesystem::path subDataSetDir, CallbackT callback){
	std::filesystem::directory_iterator iter(subDataSetDir);
	for(auto respFileCand: iter){
		if(isRespFile(respFileCand.path())){
			auto respFile = respFileCand.path();
			auto challFile = getChallFilePathFromResp(respFile);
			callback(challFile, respFile);
		}
	}
}



std::filesystem::path FileNamePairGenCompPostfix::getChallFilePathFromResp(std::filesystem::path &fn) {
	return fn.stem();
}


std::filesystem::path FileNamePairGenCompReplace::getChallFilePathFromResp(std::filesystem::path &fn) {
	std::filesystem::path res{fn};
	res.replace_extension(rawExt);
	return res;
}


std::unique_ptr<IFileNamePairGen> parserGenFactory(FTSMetadata_ptrs &parsedMetaFile){
	std::string processedExtStr = parsedMetaFile.processedExt.str;
	if(parsedMetaFile.rawExt.size){
		std::string rawExtStr = parsedMetaFile.rawExt.str;
		return std::make_unique<FileNamePairGenCompReplace>(processedExtStr, rawExtStr);
	} else {
		return std::make_unique<FileNamePairGenCompPostfix>(processedExtStr);
	}
}

void crawlTestSuite(std::filesystem::path testSuiteDirPath, IFileNamePairGen::CallbackT callback){
	std::filesystem::directory_iterator fuckingDirIter(testSuiteDirPath);
	for(auto& subTestDirCandidate: fuckingDirIter){
		if(subTestDirCandidate.is_directory()){
			std::filesystem::path candidateMetadataPath = subTestDirCandidate.path() / "meta.ftsmeta";
			if(std::filesystem::is_regular_file(candidateMetadataPath)){
				std::cout << candidateMetadataPath << '\n';

				{
					std::unique_ptr<IFileNamePairGen> gen;
					{
						mio::basic_mmap_source<uint8_t> ro_mmap(std::string(candidateMetadataPath), 0, mio::map_entire_file);
						pseudospan mappedPspan((uint8_t*) ro_mmap.data(), ro_mmap.size());

						struct FTSMetadata_ptrs parsedMetadata;

						Metadata_parse(&(*mappedPspan.begin()), &(*mappedPspan.end()), &parsedMetadata);
						//std::cout << "parsedMetadata.rawExt.str " << parsedMetadata.rawExt.str << std::endl;
						//std::cout << "parsedMetadata.processedExt.str " << parsedMetadata.processedExt.str << std::endl;

						gen = parserGenFactory(parsedMetadata);
					}

					gen->processChallengeResponseFilePairs(subTestDirCandidate, callback);
				}

			}
		}
	}
}


FTSTestToolkit::FTSTestToolkit(ITestToolkit* tk):tk(tk){}

void FTSTestToolkit::processOutOfBound(const char * ptrDescription, const char * whichBound, uint8_t * bound_ptr){
	std::stringstream res;
	res << "Out of bound access, (";
	res << ptrDescription;
	res << " = " << bound_ptr << ") > (" << whichBound << " bound = " << bound_ptr << ")" << std::endl;
	tk->reportError(res.str());
}

void FTSTestToolkit::processInequality(size_t chunkSpanRelativeBegin, size_t chunkSpanRelativeEnd){
	std::stringstream res;
	res << "Decompressed buffer is not equal to compressed one, chunk " << chunkSpanRelativeBegin << ", "<< chunkSpanRelativeEnd << std::endl;
	tk->reportError(res.str());
}

using TesteeSpecificContextSharedLibFactoryT = SharedLibFactory<TesteeSpecificContextFactoryT>;
using TesteeSpecificContextSharedLibFactoryPtrT = std::unique_ptr<TesteeSpecificContextSharedLibFactoryT>;

struct OurProtoTest : public IProtoTest{
	std::filesystem::path chall, resp;
	TesteeSpecificContextSharedLibFactoryT& testeeContextFactory;

	OurProtoTest(TesteeSpecificContextSharedLibFactoryT& testeeContextFactory, std::filesystem::path chall, std::filesystem::path resp): IProtoTest(chall, chall.stem(), chall.parent_path().filename(), 0), chall(chall), resp(resp), testeeContextFactory(testeeContextFactory){
		//std::cout << "chall " << chall << " " << std::string(chall.stem()) << std::endl;
		//std::cout << "resp " << resp << " " << resp.stem() << std::endl;
	}

	void operator()(ITestToolkit * tk) {
		TesteeSpecificContext *tctx = testeeContextFactory();
		FTSTestToolkit ftk(tk);

		std::string challengeFilePath = chall;
		std::string responseFilePath = resp;

		mio::basic_mmap_source<uint8_t> response_data_map(responseFilePath, 0, mio::map_entire_file);
		pseudospan response_data_span((uint8_t*) response_data_map.data(), response_data_map.size());

		mio::basic_mmap_source<uint8_t> challenge_data_map(challengeFilePath, 0, mio::map_entire_file);
		pseudospan challenge_data_span((uint8_t*) challenge_data_map.data(), challenge_data_map.size());

		if(tctx->shouldSwapChallengeResponse){
			std::swap(response_data_span, challenge_data_span);
		}

		ftk.tk->assertDelayedSuccess();
		{
			auto outBuffTester = tctx->makeOutputBufferTester(response_data_span, &ftk);
			tctx->executeTestee(challenge_data_span, *outBuffTester);
		}
		this->verifyTestingOutput(&ftk);
		tctx->verifyAxillaryResults(&ftk);
	}
	void verifyTestingOutput(FTSTestToolkit *ftk){
		ftk->tk->assertDelayedSuccess();
	}
};

std::unique_ptr<OutputBufferTester> TesteeSpecificContext::makeOutputBufferTester(pseudospan &etalonSpan, FTSTestToolkit * ctx){
	return std::make_unique<OutputBufferTester>(etalonSpan, ctx);
}

OutputBufferTester::~OutputBufferTester() = default;

struct ArgumentMissingException: public std::logic_error
{
	ArgumentMissingException(const std::string& what): std::logic_error(what){}
};

ITestsGenerator::~ITestsGenerator() = default;

class FTSITestsGenerator: public ITestsGenerator{
	std::filesystem::path testSuiteDir;
	std::unique_ptr<TesteeSpecificContextSharedLibFactoryT> testeeContextFactory;

	virtual void parseUserCLIArgs(ITestCLIInitializer * ti, int argc, const char **argv){
		HydrArgs::SArg<HydrArgs::ArgType::string> testLibNameA{'t', "test_lib", "a library implementing your testing routine", 0, "Path to the lib"};
		HydrArgs::SArg<HydrArgs::ArgType::string> fileTestSuiteDir{'s', "suite_dir", "a dir with the tests suite", 0, "Path to the dir", nullptr, "../../tests/testDataset"};

		std::vector<HydrArgs::Arg*> dashedSpec{};
		std::vector<HydrArgs::Arg*> positionalSpec{&testLibNameA, &fileTestSuiteDir};

		std::unique_ptr<HydrArgs::IArgsParser> ap{HydrArgs::Backend::argsParserFactory("FileTestSuite generator", "This program generates test cases and runs them from FileTestsSuite specs", "", dashedSpec, positionalSpec)};

		if(!ap){
			std::cerr << "Cannot initialize CLI parsing library!" << std::endl;
		}

		std::cout << "Parsing CLI args" << std::endl;
		HydrArgs::ParseResult res = (*ap)(HydrArgs::CLIRawArgs{.argv0=argv[0], .argvRest={&argv[1], static_cast<size_t>(argc - 1)}});

		if(res.parsingStatus){
			std::cout << "Error parsing CLI args, error code " << res.parsingStatus << std::endl;
			//return res.parsingStatus.returnCode;
		}

		std::cout << "Test suite dir: " << fileTestSuiteDir.value << std::endl;
		std::cout << "Test lib: " << testLibNameA.value << std::endl;
		testSuiteDir = std::filesystem::path(fileTestSuiteDir.value);

		auto &testLibName = testLibNameA.value;
		testeeContextFactory = std::make_unique<TesteeSpecificContextSharedLibFactoryT>(const_cast<const char *>(testLibName.data()), "testeeSpecificContextFactory");
	}

	virtual int operator()(ITestCLIInitializer * ti){
		crawlTestSuite(testSuiteDir, [&](std::filesystem::path chall, std::filesystem::path resp){
			ti->addTest(std::make_shared<OurProtoTest>(*testeeContextFactory, chall, resp));
		});
		return 0;
	}
};

ITestsGenerator *  TestsGeneratorFactory(){
	return new FTSITestsGenerator();
}


