//
// Created by mortacious on 12/30/21.
//

#pragma once
#include <vtkSmartPointer.h>
#include <vtkWrappingHints.h>
#include <vtkObject.h>
#include <vtkSetGet.h>
#include <vtkCommand.h>
#include <vtkBoundingBox.h>
#include <mutex>
#include <functional>
#include <condition_variable>
#include "lruCache.h"
#include "priorityQueue.h"
#include "vtkTileHierarchyModule.h" // For export macro

class vtkMapper;
class vtkTileHierarchyNode;
using vtkTileHierarchyNodePtr = vtkSmartPointer<vtkTileHierarchyNode>;

// custom specialization of std::hash for node pointers
template<>
struct std::hash<vtkTileHierarchyNodePtr> {
    std::size_t operator()(vtkTileHierarchyNodePtr const &s) const noexcept;
};

VTK_WRAPEXCLUDE struct vtkTileHierarchyLoaderRenderState {
    vtkTileHierarchyLoaderRenderState() = default;
    virtual ~vtkTileHierarchyLoaderRenderState() = default;
};

class VTKTILEHIERARCHY_EXPORT vtkTileHierarchyLoaderBase: public vtkObject {
private:
    using OnNodeLoadedFunction = std::function<void(vtkTileHierarchyNodePtr&)>;
public:
    vtkTypeMacro(vtkTileHierarchyLoaderBase, vtkObject);
    void PrintSelf(ostream& os, vtkIndent indent) override;

    virtual void Initialize();
    virtual void Shutdown();

    const vtkBoundingBox& GetBoundingBox() const {
        return BoundingBox;
    }

    void UnscheduleAll();
    void ScheduleForLoading(vtkTileHierarchyNodePtr& node, float priority);
    virtual void LoadNode(vtkTileHierarchyNodePtr node) = 0;

    virtual std::unique_ptr<vtkTileHierarchyLoaderRenderState> PreRender() {
        return std::make_unique<vtkTileHierarchyLoaderRenderState>();
    };

    virtual void PostRender(std::unique_ptr<vtkTileHierarchyLoaderRenderState> state) {
    };

    vtkGetMacro(Initialized, bool);

    void SetCacheSize(size_t cs) {
        Cache.max_cache_size(cs);
    }

    size_t GetCacheSize() const {
        return Cache.cache_size();
    }

    vtkTileHierarchyNode* GetRootNode();
    void SetRootNode(vtkTileHierarchyNode* root_node);

    void UnloadNode(vtkTileHierarchyNodePtr& node) {
        UnloadNode(node, false);
    }

    void UnloadNode(vtkTileHierarchyNodePtr& node, bool recursive);

    void SetNodeLoadedCallBack(OnNodeLoadedFunction func);

    vtkGetMacro(Stop, bool);

protected:
    vtkTileHierarchyLoaderBase();
    ~vtkTileHierarchyLoaderBase() noexcept override;

    bool TryGetNodeFromCache(vtkTileHierarchyNodePtr& node);
    void InvokeNodeLoaded(vtkTileHierarchyNodePtr& node) const;

    vtkTileHierarchyNodePtr PopNextNode();
    virtual void GetNode(vtkTileHierarchyNodePtr& node, bool recursive);

    virtual void RunOnce();
    virtual void Run();

    virtual void DoInitialize() {}
    virtual void DoShutdown() {}

    bool Initialized;
    std::atomic_bool Stop;
    mutable std::mutex Mutex;
    std::condition_variable Cond;

    PriorityQueue<vtkTileHierarchyNodePtr , float> NeedToLoad;

    vtkTileHierarchyNodePtr RootNode;

    struct TileTreeNodeSize {
        size_t operator()(const vtkTileHierarchyNodePtr & k , const std::pair<vtkSmartPointer<vtkMapper>, size_t>& v) const;
    };

    using LRUCacheType = LRUCache<vtkTileHierarchyNodePtr, std::pair<vtkSmartPointer<vtkMapper>, size_t>, TileTreeNodeSize>;
    LRUCacheType Cache;

    std::mutex CacheMutex;

    OnNodeLoadedFunction Func;
    vtkBoundingBox BoundingBox;
private:
    vtkTileHierarchyLoaderBase(const vtkTileHierarchyLoaderBase&) = delete;
    void operator=(const vtkTileHierarchyLoaderBase&) = delete;
};

