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
#include "minMaxHeap.h"
#include "lruCache.h"
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

    vtkGetMacro(MaxInQueue, unsigned int);
    vtkSetMacro(MaxInQueue, unsigned int);

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

    void SetNodeLoadedCallBack(const std::function<void()>& func);

    vtkGetMacro(Stop, bool);

protected:
    vtkTileHierarchyLoaderBase();
    ~vtkTileHierarchyLoaderBase() noexcept override;

    bool TryGetNodeFromCache(vtkTileHierarchyNodePtr& node);
    void InvokeNodeLoaded() const;

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

    using HeapElement = std::pair<vtkTileHierarchyNodePtr, float>;

    struct Compare
    {
        bool operator()(const HeapElement& e1, const HeapElement& e2) const
        {
            return e1.second < e2.second;
        }
    };
    minmax::MinMaxHeap<HeapElement, std::vector<HeapElement>, Compare> NeedToLoad;

    unsigned int MaxInQueue;

    vtkTileHierarchyNodePtr RootNode;

    struct TileTreeNodeSize {
        size_t operator()(const vtkTileHierarchyNodePtr & k , const std::pair<vtkSmartPointer<vtkMapper>, size_t>& v) const;
    };

    using LRUCacheType = LRUCache<vtkTileHierarchyNodePtr, std::pair<vtkSmartPointer<vtkMapper>, size_t>, TileTreeNodeSize>;
    LRUCacheType Cache;

    std::mutex CacheMutex;

    std::function<void()> Func;
    vtkBoundingBox BoundingBox;
private:
    vtkTileHierarchyLoaderBase(const vtkTileHierarchyLoaderBase&) = delete;
    void operator=(const vtkTileHierarchyLoaderBase&) = delete;
};

