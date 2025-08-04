#include "MemoryAllocator.h"

PagingAllocator::PagingAllocator(size_t maxMemorySize, size_t numFrames) 
	: maxMemorySize(maxMemorySize), numFrames(numFrames) {
	// initialize free frame list
	for (size_t i = 0; i < numFrames; ++i) {
		freeFrameList.push_back(i);
	}
}

void* PagingAllocator::allocate(std::shared_ptr<Process> process) {
	size_t processID = process->getPID();
	size_t requiredFrames = process->getNumPages();

	if (requiredFrames > freeFrameList.size()) {
		std::cerr << "Memory allocation failed. Not enough free frames.\n";
		return nullptr;
	}

	// Allocate frames for the process
	size_t frameIndex = allocateFrames(requiredFrames, processID);
	return reinterpret_cast<void*>(frameIndex);
}

void PagingAllocator::deallocate(std::shared_ptr<Process> process) {
	size_t processID = process->getPID();

	// Find and free all frames allocated to this process
	auto it = std::find_if(frameMap.begin(), frameMap.end(),
		[processID](const auto& entry) { return entry.second == processID; });

	while (it != frameMap.end()) {
		size_t frameIndex = it->first;
		deallocateFrames(1, frameIndex);
		it = std::find_if(frameMap.begin(), frameMap.end(),
			[processID](const auto& entry) { return entry.second == processID; });
	}
}

std::string PagingAllocator::visualizeMemory() {
	std::ostringstream oss;
	oss << "Paging Memory Visualization:\n";
	for (size_t frameIndex = 0; frameIndex < numFrames; ++frameIndex) {
		auto it = frameMap.find(frameIndex);
		if (it != frameMap.end()) {
			oss << "Frame " << frameIndex << " -> Process ID " << it->second << "\n";
		} else {
			oss << "Frame " << frameIndex << " -> Free\n";
		}
	}
	oss << "=========================\n";
	return oss.str();
}

size_t PagingAllocator::allocateFrames(size_t numFrames, size_t processID) {
	size_t frameIndex = freeFrameList.back();
	freeFrameList.pop_back();

	// Map allocared frames to the process ID
	for (size_t i = 0; i < numFrames; ++i) {
		frameMap[frameIndex + i] = processID;
	}
	return frameIndex;
}

void PagingAllocator::deallocateFrames(size_t numFrames, size_t frameIndex) {
	// Remove mapping 
	for (size_t i = 0; i < numFrames; ++i) {
		frameMap.erase(frameIndex + i);
	}

	// Add back to free frame list
	for (size_t i = 0; i < numFrames; ++i) {
		freeFrameList.push_back(frameIndex + i);
	}
}