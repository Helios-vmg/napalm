void metadata_destroy(MetadataPtr instance){
	delete (OggMetadataWrapper *)instance;
}

void metadata_release_buffer(MetadataBuffer buffer){
	delete[] buffer.ptr;
}

MetadataBuffer to_MetadataBuffer(const std::string &s){
	MetadataBuffer ret;
	ret.size = s.size();
	size_t size = ret.size ? ret.size : 1;
	ret.ptr = new std::uint8_t[size];
	if (ret.size)
		memcpy(ret.ptr, &s[0], ret.size);
	return ret;
}

MetadataBuffer to_MetadataBuffer(const std::vector<std::uint8_t> &v){
	MetadataBuffer ret;
	ret.size = v.size();
	size_t size = ret.size ? ret.size : 1;
	ret.ptr = new std::uint8_t[size];
	if (ret.size)
		memcpy(ret.ptr, &v[0], ret.size);
	return ret;
}

#define DEFINE_METADATA_BUFFER_GETTER(x) \
	MetadataBuffer metadata_get_##x(MetadataPtr instance){ \
		auto &metadata = *(OggMetadataWrapper *)instance; \
		auto &module = metadata.decoder->get_module(); \
		try{ \
			return to_MetadataBuffer(metadata.metadata.x()); \
		}catch (std::exception &e){ \
			module.set_error(e.what()); \
		} \
		return { nullptr, 0 }; \
	}

#define DEFINE_METADATA_DOUBLE_GETTER(x) \
	double metadata_get_##x(MetadataPtr instance){ \
		auto &metadata = *(OggMetadataWrapper *)instance; \
		auto &module = metadata.decoder->get_module(); \
		try{ \
			return metadata.metadata.x(); \
		}catch (std::exception &e){ \
			module.set_error(e.what()); \
		} \
		return std::nan(""); \
	}

DEFINE_METADATA_BUFFER_GETTER(album)
DEFINE_METADATA_BUFFER_GETTER(track_title)
DEFINE_METADATA_BUFFER_GETTER(track_number)
DEFINE_METADATA_BUFFER_GETTER(track_artist)
DEFINE_METADATA_BUFFER_GETTER(date)
DEFINE_METADATA_BUFFER_GETTER(front_cover)
DEFINE_METADATA_DOUBLE_GETTER(track_gain)
DEFINE_METADATA_DOUBLE_GETTER(track_peak)
DEFINE_METADATA_DOUBLE_GETTER(album_gain)
DEFINE_METADATA_DOUBLE_GETTER(album_peak)

typedef std::pair<OggMetadataIterator, Module *> Iterator;

MetadataIteratorPtr metadata_get_iterator(MetadataPtr instance){
	auto &metadata = *(OggMetadataWrapper *)instance;
	auto &module = metadata.decoder->get_module();
	try{
		auto ret = metadata.metadata.get_iterator(&module).release();
		static_assert(std::is_same<decltype(ret), Iterator *>::value, "");
		return ret;
	}catch (std::exception &e){
		module.set_error(e.what());
	}
	return nullptr;
}

void metadata_iterator_release(MetadataIteratorPtr instance){
	delete (Iterator *)instance;
}

int metadata_iterator_get_next(MetadataIteratorPtr instance, MetadataKeyValue *kv){
	auto &iterator = *(Iterator *)instance;
	auto &module = *iterator.second;
	MetadataBuffer mb_key{ nullptr, 0 },
		mb_value{ nullptr, 0 };
	try{
		std::string key, value;
		if (!iterator.first.next(key, value))
			return 0;
		mb_key = to_MetadataBuffer(key);
		mb_value = to_MetadataBuffer(value);
		kv->key = mb_key;
		kv->value = mb_value;
		return 1;
	}catch (std::exception &e){
		module.set_error(e.what());
		metadata_release_buffer(mb_key);
		metadata_release_buffer(mb_value);
	}
	return -1;
}
