//
//  fountain-encoder.hpp
//
//  Copyright © 2020 by Blockchain Commons, LLC
//  Licensed under the "BSD-2-Clause Plus Patent License"
//

#ifndef BC_UR_FOUNTAIN_ENCODER_HPP
#define BC_UR_FOUNTAIN_ENCODER_HPP

#include <stddef.h>
#include <vector>
#include <exception>
#include "utils.hpp"
#include "fountain-utils.hpp"

namespace ur {

// Implements Luby transform code rateless coding
// https://en.wikipedia.org/wiki/Luby_transform_code

class FountainEncoder final {
public:
    class Part {
    public:
        class InvalidHeader: public std::exception { };

        Part(uint32_t seq_num, size_t seq_len, size_t message_len, uint32_t checksum, const ByteVector& data) 
            : seq_num_(seq_num), seq_len_(seq_len), message_len_(message_len), checksum_(checksum), data_(data)
        { }
        //读取cbor数据格式，并以其内容填充类的私有成员变量
        explicit Part(const ByteVector& cbor);

        uint32_t seq_num() const { return seq_num_; }
        size_t seq_len() const { return seq_len_; }
        size_t message_len() const { return message_len_; }
        uint32_t checksum() const { return checksum_; }
        const ByteVector& data() const { return data_; }

        //将part中的私有成员编码为cbor格式的数据，其中最外层是arrysize=5的数组
        ByteVector cbor() const;
        //打印的一个格式，将所有的私有变量读取，并将结果返回为字符串
        std::string description() const;

    private:
        uint32_t seq_num_;
        size_t seq_len_;
        size_t message_len_;
        uint32_t checksum_;
        ByteVector data_;
    };

    FountainEncoder(const ByteVector& message, size_t max_fragment_len, uint32_t first_seq_num = 0, size_t min_fragment_len = 10);
    
    //计算得到最接近max_fragment_len的一个fragment的大小，然后返回这个值
    static size_t find_nominal_fragment_length(size_t message_len, size_t min_fragment_len, size_t max_fragment_len);
    //将message按照fragment_len的长度分成多个fragments进行返回
    static std::vector<ByteVector> partition_message(const ByteVector &message, size_t fragment_len);

    uint32_t seq_num() const { return seq_num_; }
    const PartIndexes& last_part_indexes() const { return last_part_indexes_; }
    size_t seq_len() const { return fragments_.size(); }

    // This becomes `true` when the minimum number of parts
    // to relay the complete message have been generated
    bool is_complete() const { return seq_num_ >= seq_len(); }

    /// True if only a single part will be generated.
    bool is_single_part() const { return seq_len() == 1; }

    Part next_part();

private:
    size_t message_len_;
    uint32_t checksum_;
    size_t fragment_len_;
    std::vector<ByteVector> fragments_;
    uint32_t seq_num_;
    PartIndexes last_part_indexes_;

    ByteVector mix(const PartIndexes& indexes) const;
};

}

#endif // BC_UR_FOUNTAIN_ENCODER_HPP
