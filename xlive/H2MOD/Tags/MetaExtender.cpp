#include "MetaExtender.h"
#include "H2MOD/Modules/Config/Config.h"
#include "Blam/Cache/TagGroups/projectile_definition.hpp"
#include "H2MOD/Modules/Utils/Utils.h"
#include "Blam/Cache/TagGroups/biped_definition.hpp"
#include "type_traits"
#include <typeinfo>
#include <typeindex>
#include "Blam/Cache/TagGroups/scenario_definition.hpp"
std::vector<void*> ToFree;


namespace MetaExtender {
	template<typename T = void>
	void move_child_block(unsigned long tag_block_ptr)
	{
		int* block_count = reinterpret_cast<int*>(tag_block_ptr);
		int* block_offset = reinterpret_cast<int*>(tag_block_ptr + 4);
		size_t block_size = *block_count * sizeof(T);
		void* new_memory = calloc(*block_count + 1, sizeof(T));
		ToFree.push_back(new_memory);
		memcpy(new_memory, &tags::get_tag_data()[*block_offset], block_size);
		if (visit_struct::traits::is_visitable<T>::value)
		{
			for (auto i = 0; i < *block_count; i++)
			{
				const T* t_block = reinterpret_cast<T*>(((unsigned long)new_memory) + (sizeof(T) * i));
				visit_struct::for_each(*t_block, [](const char *name, const auto &value)
				{
					if (std::string(typeid(value).name()).find("tag_block") != std::string::npos)
					{
						auto a = reinterpret_cast<decltype(value)>(value);
						LOG_INFO_GAME(IntToString<unsigned long>((unsigned long)std::addressof(value), std::hex));

						if (a.size != 0)
						{
							move_child_block<decltype(a[0])>((unsigned long)std::addressof(value));
						}
					}
				});
			}
		}
	}
	//Currently only works with 1 level deep reflexives.
	void add_tag_block(unsigned long tag_block_ptr, int tag_block_item_size, int tag_block_item_count, int newCount)
	{
		size_t currentSize = tag_block_item_size * tag_block_item_count;

		void* new_tag_block = calloc(tag_block_item_count + newCount, tag_block_item_size);

		ToFree.push_back(new_tag_block);

		memcpy(new_tag_block, &tags::get_tag_data()[*reinterpret_cast<int*>(tag_block_ptr + 4)], currentSize);

		*reinterpret_cast<int*>(tag_block_ptr) = tag_block_item_count + newCount;
		*reinterpret_cast<int*>(tag_block_ptr + 4) = (unsigned long)new_tag_block - int(*h2mod->GetAddress<int**>(0x47CD54));
	}


	template<typename T = void>
	T* add_tag_block2(unsigned long tag_block_ptr)
	{
		int* block_count = reinterpret_cast<int*>(tag_block_ptr);
		int* block_offset = reinterpret_cast<int*>(tag_block_ptr + 4);
		size_t block_size = *block_count * sizeof(T);
		void* new_memory = calloc(*block_count + 1, sizeof(T));
		ToFree.push_back(new_memory);

		memcpy(new_memory, &tags::get_tag_data()[*block_offset], block_size);

		*block_count = *block_count + 1;
		*block_offset = (int)((unsigned long)new_memory - int(*h2mod->GetAddress<int**>(0x47CD54)));

		if (visit_struct::traits::is_visitable<T>::value)
		{
			for (auto i = 0; i < *block_count; i++)
			{
				const T* t_block = reinterpret_cast<T*>(((unsigned long)new_memory) + (sizeof(T) * i));
				/*for(static int j = 0; j < visit_struct::field_count(t_block); j++)
				{
					static int index = j;
					auto t_name = typeid(visit_struct::type_at<index, T>()).name();
					if(strstr(t_name, "tag_block") != NULL)
					{
						auto v_t_block = reinterpret_cast<visit_struct::type_at<index, T>()>(visit_struct::get_pointer<index, T>());
						LOG_INFO_GAME(std::to_string(v_t_block.size));
					}
				}*/
				//visit_struct::for_each(*t_block, [](const char *name, const auto &value)
				//{
				//	if(std::string(typeid(value).name()).find("tag_block") != std::string::npos)
				//	{
				//		auto a = reinterpret_cast<decltype(value)>(value);
				//		int* block_size = reinterpret_cast<int*>((unsigned long)std::addressof(value));
				//		LOG_INFO_GAME(IntToString<unsigned long>((unsigned long)std::addressof(value), std::hex));

				//		if(value.size != 0)
				//		{
				//			visit_struct::get<>()
				//			move_child_block<decltype(value[0])>((unsigned long)std::addressof(value));
				//		}
				//	}
				//});
			}
		}

		return reinterpret_cast<T*>(((unsigned long)new_memory) + (sizeof(T) * (*block_count - 1)));
	}
	template<typename T = void>
	T* add_tag_block3(tag_block<T>* block)
	{
		size_t block_size = block->size * sizeof(T);
		void* new_memory = calloc(block->size + 1, sizeof(T));
		ToFree.push_back(new_memory);

		memcpy(new_memory, &tags::get_tag_data()[block->data], block_size);

		block->size = block->size + 1;
		block->data = (int)((unsigned long)new_memory - int(*h2mod->GetAddress<int**>(0x47CD54)));

		return reinterpret_cast<T*>(((unsigned long)new_memory) + (sizeof(T) * (block->size - 1)));
	}
	template<typename T = void>
	T* add_tag_block4(tag_block<T>* block)
	{
		
		size_t block_size = block->size * sizeof(T);
		void* new_memory = calloc(block->size + 1, sizeof(T));
		ToFree.push_back(new_memory);

		memcpy(new_memory, &tags::get_tag_data()[block->data], block_size);

		block->size = block->size + 1;
		block->data = (int)((unsigned long)new_memory - int(*h2mod->GetAddress<int**>(0x47CD54)));

		if (visit_struct::traits::is_visitable<T>::value)
		{
			for(auto i = 0; i < (int)block->size; i++)
			{
				const T t_block = 
				visit_struct::for_each(&t_block, [](const char *name, visit_struct::type_c<tag_block<void*>>)
				{
					if(dynamic_cast<const tag_block<void*>*>(value) != nullptr)
					{
					}
				});
			}
		}

		return reinterpret_cast<T*>(((unsigned long)new_memory) + (sizeof(T) * (block->size - 1)));
	}

	void test()
	{
		/*auto bull = tags::find_tag(blam_tag::tag_group_type::biped, "objects\\characters\\masterchief\\masterchief_mp");
		auto t_bull = tags::get_tag<blam_tag::tag_group_type::biped, s_biped_group_definition>(bull);*/
		auto scnr = tags::find_tag(blam_tag::tag_group_type::scenario, "scenarios\\multi\\cyclotron\\cyclotron");
		auto t_scnr = tags::get_tag<blam_tag::tag_group_type::scenario, s_scenario_group_definition>(scnr);
		auto new_decoration = MetaExtender::add_tag_block2<s_scenario_group_definition::s_decorators_block>((unsigned long)std::addressof(t_scnr->decals));
		//t_bull->initial_velocity = 1;
		//t_bull->final_velocity = 1;
		///*MetaExtender::add_tag_block((unsigned long)std::addressof(t_bull->objectTag.attachments), 0x18, t_bull->objectTag.attachments.size, 1);
		//t_bull->objectTag.attachments[1]->type.TagGroup = blam_tag::tag_group_type::light;
		//t_bull->objectTag.attachments[1]->type.TagIndex = datum(0xEDF83393);*/
		//auto new_attachment = MetaExtender::add_tag_block2<s_object_group_definition::s_change_colors_block>((unsigned long)std::addressof(t_bull->unitTag.objectTag.change_colors));
		//auto new_attachment = MetaExtender::add_tag_block2<s_object_group_definition::s_attachments_block>(&t_bull->objectTag.attachments);
		//new_attachment->type.TagGroup = blam_tag::tag_group_type::light;
		//new_attachment->type.TagIndex = datum(0xEDF83393);
	}

	void free_tag_blocks()
	{
		for (auto &block : ToFree)
		{
			free(block);
		}
		ToFree.clear();
	}

}