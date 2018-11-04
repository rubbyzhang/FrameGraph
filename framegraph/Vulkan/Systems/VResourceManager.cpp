// Copyright (c) 2018,  Zhirnov Andrey. For more information see 'LICENSE'

#include "VResourceManager.h"
#include "VDevice.h"

namespace FG
{

/*
=================================================
	constructor
=================================================
*/
	VResourceManager::VResourceManager (const VDevice &dev) :
		_device{ dev }
	{
		_readyToDelete.reserve( 256 );
	}
	
/*
=================================================
	destructor
=================================================
*/
	VResourceManager::~VResourceManager ()
	{
		ASSERT( _readyToDelete.empty() );
	}
	
/*
=================================================
	OnBeginFrame
=================================================
*/
	void VResourceManager::OnBeginFrame ()
	{
	}
	
/*
=================================================
	OnEndFrame
=================================================
*/
	void VResourceManager::OnEndFrame ()
	{
		_DeleteResources();
		_UnassignResourceIDs();
	}
	
/*
=================================================
	_UnassignResourceIDs
=================================================
*/
	void VResourceManager::_UnassignResourceIDs ()
	{
		for (auto& vid : _unassignIDs)
		{
			std::visit( [this] (auto id) { _UnassignResource( _GetResourcePool(id), id ); }, vid );
		}
		_unassignIDs.clear();
	}
	
/*
=================================================
	_DeleteResources
=================================================
*/
	void VResourceManager::_DeleteResources ()
	{
		VkDevice	dev = _device.GetVkDevice();
		
		for (auto& pair : _readyToDelete)
		{
			switch ( pair.first )
			{
				case VK_DEBUG_REPORT_OBJECT_TYPE_SEMAPHORE_EXT :
					_device.vkDestroySemaphore( dev, VkSemaphore(pair.second), null );
					break;

				case VK_DEBUG_REPORT_OBJECT_TYPE_FENCE_EXT :
					_device.vkDestroyFence( dev, VkFence(pair.second), null );
					break;

				case VK_DEBUG_REPORT_OBJECT_TYPE_DEVICE_MEMORY_EXT :
					_device.vkFreeMemory( dev, VkDeviceMemory(pair.second), null );
					break;

				case VK_DEBUG_REPORT_OBJECT_TYPE_IMAGE_EXT :
					_device.vkDestroyImage( dev, VkImage(pair.second), null );
					break;

				case VK_DEBUG_REPORT_OBJECT_TYPE_EVENT_EXT :
					_device.vkDestroyEvent( dev, VkEvent(pair.second), null );
					break;

				case VK_DEBUG_REPORT_OBJECT_TYPE_QUERY_POOL_EXT :
					_device.vkDestroyQueryPool( dev, VkQueryPool(pair.second), null );
					break;

				case VK_DEBUG_REPORT_OBJECT_TYPE_BUFFER_EXT :
					_device.vkDestroyBuffer( dev, VkBuffer(pair.second), null );
					break;

				case VK_DEBUG_REPORT_OBJECT_TYPE_BUFFER_VIEW_EXT :
					_device.vkDestroyBufferView( dev, VkBufferView(pair.second), null );
					break;

				case VK_DEBUG_REPORT_OBJECT_TYPE_IMAGE_VIEW_EXT :
					_device.vkDestroyImageView( dev, VkImageView(pair.second), null );
					break;

				case VK_DEBUG_REPORT_OBJECT_TYPE_PIPELINE_LAYOUT_EXT :
					_device.vkDestroyPipelineLayout( dev, VkPipelineLayout(pair.second), null );
					break;

				case VK_DEBUG_REPORT_OBJECT_TYPE_RENDER_PASS_EXT :
					_device.vkDestroyRenderPass( dev, VkRenderPass(pair.second), null );
					break;

				case VK_DEBUG_REPORT_OBJECT_TYPE_PIPELINE_EXT :
					_device.vkDestroyPipeline( dev, VkPipeline(pair.second), null );
					break;

				case VK_DEBUG_REPORT_OBJECT_TYPE_DESCRIPTOR_SET_LAYOUT_EXT :
					_device.vkDestroyDescriptorSetLayout( dev, VkDescriptorSetLayout(pair.second), null );
					break;

				case VK_DEBUG_REPORT_OBJECT_TYPE_SAMPLER_EXT :
					_device.vkDestroySampler( dev, VkSampler(pair.second), null );
					break;

				case VK_DEBUG_REPORT_OBJECT_TYPE_DESCRIPTOR_POOL_EXT :
					_device.vkDestroyDescriptorPool( dev, VkDescriptorPool(pair.second), null );
					break;

				case VK_DEBUG_REPORT_OBJECT_TYPE_FRAMEBUFFER_EXT :
					_device.vkDestroyFramebuffer( dev, VkFramebuffer(pair.second), null );
					break;

				case VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_POOL_EXT :
					_device.vkDestroyCommandPool( dev, VkCommandPool(pair.second), null );
					break;

				case VK_DEBUG_REPORT_OBJECT_TYPE_SAMPLER_YCBCR_CONVERSION_EXT :
					_device.vkDestroySamplerYcbcrConversion( dev, VkSamplerYcbcrConversion(pair.second), null );
					break;

				case VK_DEBUG_REPORT_OBJECT_TYPE_DESCRIPTOR_UPDATE_TEMPLATE_EXT :
					_device.vkDestroyDescriptorUpdateTemplate( dev, VkDescriptorUpdateTemplate(pair.second), null );
					break;

				case VK_DEBUG_REPORT_OBJECT_TYPE_ACCELERATION_STRUCTURE_NVX_EXT :
					_device.vkDestroyAccelerationStructureNVX( dev, VkAccelerationStructureNVX(pair.second), null );
					break;

				default :
					FG_LOGE( "resource type is not supported" );
					break;
			}
		}
		_readyToDelete.clear();
	}
	
/*
=================================================
	_DestroyResourceCache
=================================================
*/
	template <typename DataT, size_t CS, size_t MC>
	void VResourceManager::_DestroyResourceCache (CachedPoolTmpl<DataT,CS,MC> &res)
	{
		for (size_t i = 0, count = res.size(); i < count; ++i)
		{
			Index_t	id	 = Index_t(i);
			auto&	data = res[id];
				
			if ( //res.IsAssigned( id ) and
				 data.GetState() != ResourceBase::EState::Initial )
			{
				data.Destroy( OUT _readyToDelete, OUT _unassignIDs );
				res.RemoveFromCache( id );
				res.Unassign( id );
			}
		}
	}
	
/*
=================================================
	OnDestroy
=================================================
*/
	void VResourceManager::OnDestroy ()
	{
		_DestroyResourceCache( _samplerCache );
		_DestroyResourceCache( _pplnLayoutCache );
		_DestroyResourceCache( _dsLayoutCache );

		OnEndFrame();
	}


}	// FG