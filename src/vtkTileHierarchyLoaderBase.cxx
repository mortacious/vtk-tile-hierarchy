//
// Created by mortacious on 12/30/21.
//

#include "vtkTileHierarchyLoaderBase.h"
#include "vtkTileHierarchyNode.h"
#include <vtkObjectFactory.h>
#include <vtkMapper.h>
#include <limits>

std::size_t std::hash<vtkTileHierarchyNodePtr>::operator()(const vtkTileHierarchyNodePtr &s) const noexcept {
    return std::hash<vtkTileHierarchyNode *>{}(s.GetPointer());
}

size_t vtkTileHierarchyLoaderBase::TileTreeNodeSize::operator()(const vtkTileHierarchyNodePtr &k, const std::pair<vtkSmartPointer<vtkMapper>, size_t> &v) const {
    if(!k) return 0;
    return k->GetSize();
}

vtkTileHierarchyLoaderBase::vtkTileHierarchyLoaderBase()
    : Initialized(false), Stop(true), MaxInQueue(10), Cache(15000000){}

vtkTileHierarchyLoaderBase::~vtkTileHierarchyLoaderBase() noexcept {
    vtkTileHierarchyLoaderBase::Shutdown();
}

void vtkTileHierarchyLoaderBase::Initialize() {
    if(!Initialized) {
        Stop = false;
        DoInitialize();
        Initialized = true;
    }
}

void vtkTileHierarchyLoaderBase::Shutdown() {
    if(Initialized) {
        UnscheduleAll();
        Stop = true;
        std::unique_lock<std::mutex> lock{Mutex};
        Cond.notify_all();
        lock.unlock();
        DoShutdown();
        Initialized = false;
    }
}

void vtkTileHierarchyLoaderBase::UnscheduleAll() {
    std::lock_guard<std::mutex> lock{Mutex};
    while(!NeedToLoad.empty()) {
        NeedToLoad.pop();
    }
}

void vtkTileHierarchyLoaderBase::ScheduleForLoading(vtkTileHierarchyNodePtr &node, float priority) {
    if(TryGetNodeFromCache(node)) {
        //InvokeEvent(NodeLoaded, reinterpret_cast<void*>(node.GetPointer()));
        InvokeNodeLoaded();
    } else {
        std::lock_guard<std::mutex> lock{Mutex};

        NeedToLoad.push(std::make_pair(node, priority));
        while(NeedToLoad.size() > MaxInQueue) {
            NeedToLoad.popMin(); // Remove the smallest Element
        }
        Cond.notify_one();
    }
}

vtkTileHierarchyNodePtr vtkTileHierarchyLoaderBase::PopNextNode() {
    vtkTileHierarchyNodePtr node;
    while (!Stop) {
        std::unique_lock<std::mutex> lock{Mutex};
        while(NeedToLoad.empty() && !Stop) {
            Cond.wait(lock, [&]() { return !NeedToLoad.empty() || Stop; });
        }

        if (Stop) {
            lock.unlock();
            break;
        }

        node = NeedToLoad.popMax().first;

        if (node->IsLoaded()) {
            continue; // skip already loaded nodes
        }
        lock.unlock();
        break;
    }

    return std::move(node);
}

bool vtkTileHierarchyLoaderBase::TryGetNodeFromCache(vtkTileHierarchyNodePtr &node) {
    std::lock_guard<std::mutex> cache_lock{CacheMutex};
    if(Cache.exist(node)) {
        auto val = Cache.pop(node);
        std::lock_guard<std::mutex> node_lock{node->GetMutex()};
        node->Mapper = vtkSmartPointer(std::move(val.first));
        node->Size = val.second;
        return true;
    }
    return false;
}

vtkTileHierarchyNode* vtkTileHierarchyLoaderBase::GetRootNode() {
    if(!RootNode) Initialize();
    return RootNode;
}

void vtkTileHierarchyLoaderBase::SetRootNode(vtkTileHierarchyNode *root_node) {
    Register(root_node);
    RootNode = vtkTileHierarchyNodePtr(root_node);
}


void vtkTileHierarchyLoaderBase::GetNode(vtkTileHierarchyNodePtr &node, bool recursive) {
    TryGetNodeFromCache(node);
    std::unique_lock<std::mutex> node_lock{node->GetMutex()};
    if(!node->IsLoaded()) {
        LoadNode(node);
    }
    node_lock.unlock();
    InvokeNodeLoaded();
    // recursively load all nodes below as well
    if (recursive)
    {
        for (auto& child : node->Children)
        {
            if (child)
                GetNode(child, true);
        }
    }
}

void vtkTileHierarchyLoaderBase::UnloadNode(vtkTileHierarchyNodePtr &node, bool recursive) {
    std::unique_lock<std::mutex> node_lock{node->GetMutex()};
    if(node->IsLoaded() && !Cache.exist(node)) {
        std::scoped_lock<std::mutex> cache_lock{CacheMutex};
        // cache this node if it is loaded and not in the cache
        Cache.put(node, std::make_pair(std::move(node->Mapper), node->GetSize()));
    }
    node->Mapper = nullptr; // unset the mapper if not null already
    node_lock.unlock();
    if(recursive) {
        for (auto& child : node->Children) {
            if(child) {
                UnloadNode(child, true);
            }
        }
    }
}

void vtkTileHierarchyLoaderBase::PrintSelf(ostream &os, vtkIndent indent) {
    Superclass::PrintSelf(os, indent);
}

void vtkTileHierarchyLoaderBase::SetNodeLoadedCallBack(const std::function<void()> &func) {
    Func = func;
}

void vtkTileHierarchyLoaderBase::InvokeNodeLoaded() const {
    if(Func)
        Func();
}

void vtkTileHierarchyLoaderBase::RunOnce() {
    auto node = PopNextNode();
    if(node)
        GetNode(node, false);
}

void vtkTileHierarchyLoaderBase::Run() {
    while(!Stop) {
        RunOnce();
    }
}





