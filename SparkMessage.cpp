#include "SparkMessage.hh"


    SparkMessage::SparkMessage(){
        data = {};
        split_data8={};
        split_data7={};
        //final_data={};
        cmd=0;
        sub_cmd=0;
    }
    
    void SparkMessage::start_message (byte _cmd, byte _sub_cmd){
      cmd = _cmd;
      sub_cmd = _sub_cmd;
      data = {};
      split_data8 = {};
      split_data7 = {};
      final_message = {};
      //final_data = {};
    };
    
    std::vector<ByteVector> SparkMessage::end_message(){
    //ByteVector SparkMessage::end_message(){
      /*/ SKELETON
      ByteVector chunk = {};
      std::vector<ByteVector> ret_msg;
      ret_msg.push_back(chunk);

        return ret_msg;
      /*/
      
      // determine how many chunks there are
        byte end_byte = '\x79';
        int data_len = data.size();
        int num_chunks = int ((data_len + 0x7f) / 0x80 );
        

        // split the data into chunks of maximum 0x80 bytes (still 8 bit bytes)
        // and add a chunk sub-header if a multi-chunk message
  
        for (int this_chunk=0; this_chunk <num_chunks; this_chunk++){
            int chunk_len = min (0x80, data_len - (this_chunk * 0x80));
            ByteVector data8;
            if (num_chunks > 1){
                // we need the chunk sub-header
                //TODO: Remove bytes() calls if working without
                //ByteVector this_chunk_vec = SparkHelper::bytes(this_chunk);
                //ByteVector chunk_len_vec = SparkHelper::bytes(chunk_len);

                data8.push_back(num_chunks);
                data8.push_back(this_chunk);
                data8.push_back(chunk_len);
                
                //data8 = SparkHelper::bytes(num_chunks);
                //data8.insert(data8.end(), this_chunk_vec.begin(), this_chunk_vec.end());
                //data8.insert(data8.end(), chunk_len_vec.begin(), chunk_len_vec.end());
            }
            else{
                data8 = {};
            }
            data8.insert(data8.end(),data.begin()+ (this_chunk * 0x80), data.begin() + (this_chunk * 0x80 + chunk_len));
            split_data8.push_back(data8);
        }
          
        // now we can convert this to 7-bit data format with the 8-bits byte at the front
        // so loop over each chunk
        // and in each chunk loop over every sequence of (max) 7 bytes
        // and extract the 8th bit and put in 'bit8'
        //# and then add bit8 and the 7-bit sequence to data7
  
        for (auto chunk : split_data8){

            int chunk_len = chunk.size();
            int num_seq = int ((chunk_len + 6) / 7);
            ByteVector bytes7 = {};

            
            for (int this_seq = 0; this_seq < num_seq; this_seq++){
                int seq_len = min (7, chunk_len - (this_seq * 7));
                byte bit8 = 0;
                ByteVector seq = {};
                for (int ind = 0; ind < seq_len; ind++){
                    // can change this so not [dat] and not [ x: x+1]
                    byte dat = chunk[this_seq * 7 + ind];
                    if ((dat & 0x80) == 0x80){
                        bit8 |= (1<<ind);
                    }
                    dat &= 0x7f;
                    
                    seq.push_back((byte)dat);
                }
                
                //ByteVector bit8_bytes = SparkHelper::bytes(bit8);
                /*Serial.println("bit8 bytes");
                for (auto by: bit8_bytes){
                  Serial.print(SparkHelper::intToHex(by).c_str());
                  Serial.print(" ");
                }
                Serial.println();
                */
                bytes7.push_back(bit8);
                //bytes7.insert(bytes7.end(), bit8_bytes.begin(), bit8_bytes.end());
                bytes7.insert(bytes7.end(), seq.begin(), seq.end());
            }
            split_data7.push_back(bytes7);   
        }

        
        
        // now we can create the final message with the message header and the chunk header
        ByteVector block_header = {'\x01','\xfe','\x00','\x00','\x53','\xfe'};
        ByteVector block_filler = {'\x00','\x00','\x00','\x00','\x00','\x00','\x00','\x00','\x00'};
        ByteVector chunk_header = {'\xf0','\x01','\x3a','\x15'};
        
        for (auto chunk: split_data7){
            int block_size = chunk.size() + 16 + 6 + 1;
            //ByteVector block_size_vec = SparkHelper::bytes(block_size);
            //ByteVector cmd_vec = SparkHelper::bytes(cmd);
            //ByteVector sub_cmd_vec = SparkHelper::bytes(sub_cmd);
            //TODO: Remove bytes() call if working without
            
            ByteVector header = block_header;
            header.push_back(block_size);
            //header.insert(header.end(), block_size_vec.begin(), block_size_vec.end());
            header.insert(header.end(), block_filler.begin(), block_filler.end());
            header.insert(header.end(), chunk_header.begin(), chunk_header.end());       
            header.push_back(cmd);
            header.push_back(sub_cmd);
            //header.insert(header.end(), cmd_vec.begin(), cmd_vec.end());
            //header.insert(header.end(), sub_cmd_vec.begin(), sub_cmd_vec.end());
            byte trailer = '\xf7';
            ByteVector full_chunk = header;
            full_chunk.insert(full_chunk.end(), chunk.begin(), chunk.end());
            full_chunk.push_back(trailer);
            final_message.push_back(full_chunk);
        }

        //Add a 0x79 at the end of the message (experimental)
        
        //if (int ((data_len + 1 + 0x7f) / 0x80 ) > 1){
        /*
         ByteVector last_chunk = final_message[final_message.size()-1];
          final_message.pop_back();
          last_chunk.push_back(end_byte);
          final_message.push_back(last_chunk);
        //}
        */
        // EXPERIMENT TEST
        /*
        std::vector<std::string> input_data = {"01fe000053fead000000000000000000f0013a15010124040000007f5924003037303739303600332d393441392d00343142312d41420031442d30324342004335443030373902313042616e6b2000322c2050726573106574203123302e023733546573742000446573637269700074696f6e203132012869636f6e2e70146e674a4270000003172e626961732e006e6f69736567616c7465421300114a603e0f5c2901114a003e66666602f7",
                                                "01fe000053fead000000000000000000f0013a1501011c040100114a0000040000284c41324130436f6d7043130003114a000000000123114a3f5a1c2c025b114a3e420f5c31004b6c6f6e43656e007461757253696c5876657243130011514a3f3916070111494a3e6b051f0211214a3f6666662b52006f6c616e644a435831323043150011594a3f214a4101114d4a3e0f5f3b0211594a3e214a410311494a3f2b051f0411014a3f4e5604f7",
                                                "01fe000053fead000000000000000000f0013a1501010c04020026436c6f586e657242120011594a3e4c4c4d0111214a000000002c5600696e746167654430656c61794314000b114a3e420c4a013b114a3e59191a020b114a3e570a3d034b114a3f0000002b00626961732e726530766572624317002b114a3e126e18011b114a3e506560023b114a3e13774f033b114a3e4627700433114a3f147d740503114a3f2666660613114a3e4c4cf7",
                                                "01fe000053fe1e000000000000000000f0013a150101180403024d34f779"};
        final_message.clear();
        for (auto block : input_data) {
          final_message.push_back(SparkHelper::HexToBytes(block));
        }                                            
        // END EXPERIMENT TEST
        */
        //Serial.println("Message to be sent: ");
        //SparkHelper::printDataAsHexString(final_message);
        Serial.println();
        
        return final_message;

        // To be used if flat vector needs to be processed
        /*
        ByteVector flat_vector;
        while (!final_message.empty()){
          auto& last = final_message.back();
    
          std::move(std::rbegin(last), std::rend(last), std::back_inserter(flat_vector));
          final_message.pop_back();
        }
        std::reverse(std::begin(flat_vector), std::end(flat_vector));

        /*
        Serial.println("Message to be sent: ");
        for(auto by: flat_vector){
          Serial.print(SparkHelper::intToHex(by).c_str());
        }
        Serial.println();
        
        return flat_vector;
        */
    }

    void SparkMessage::add_bytes(ByteVector bytes_8){
        for (byte by: bytes_8){
          data.push_back(by);
        }
    }
    
    void SparkMessage::add_byte(byte by){
      data.push_back(by); 
    }
    
    void SparkMessage::add_prefixed_string(std::string pack_str){
      int str_length = pack_str.size();
      ByteVector byte_pack;
      byte_pack.push_back((byte)str_length);
      byte_pack.push_back((byte)(str_length + 0xa0));
      std::copy(pack_str.begin(), pack_str.end(), std::back_inserter<ByteVector>(byte_pack));
      add_bytes (byte_pack);
      /*
      Serial.print("Added prefix string: ");
      for (auto by: byte_pack){
        Serial.print(SparkHelper::intToHex(by).c_str());
      }
      Serial.println();
      */
    }
    
    
    void SparkMessage::add_string(std::string pack_str){
      int str_length = pack_str.size();
      ByteVector byte_pack;
      byte_pack.push_back((byte)(str_length + 0xa0));
      std::copy(pack_str.begin(), pack_str.end(), std::back_inserter<ByteVector>(byte_pack));
      add_bytes (byte_pack); 
      
    }
    
    void SparkMessage::add_long_string(std::string pack_str){
      int str_length = pack_str.size();
      ByteVector byte_pack;
      byte_pack.push_back(('\xd9'));
      byte_pack.push_back((byte)str_length);
      std::copy(pack_str.begin(), pack_str.end(), std::back_inserter<ByteVector>(byte_pack));
      add_bytes(byte_pack);
    }
    
    void SparkMessage::add_float (float flt){
    
      union {
        float float_variable;
        byte temp_array[4];
      } u;
      // Overite bytes of union with float variable
      u.float_variable = flt;
      ByteVector byte_pack;
      byte_pack.push_back((byte)0xca);
      // Assign bytes to input array
      for(int i=3; i>=0; i--){
        byte_pack.push_back(u.temp_array[i]);
      }

      add_bytes(byte_pack);
      //Serial.printf("Converted %f to %02X, %02X, %02X, %02X\n", flt, byte_pack[0], byte_pack[1], byte_pack[2], byte_pack[3]);
      /*/ DEBUG
      Serial.print("Added float vector: ");
      SparkHelper::printByteVector(byte_pack);
      Serial.println();
      /*/
    }
    
    void SparkMessage::add_onoff (boolean enable){
      byte b;
      if (enable == true){
            b = '\xc3';
      }
      else{
            b = '\xc2';
      }
      add_byte(b);  
    }

    std::vector<ByteVector> SparkMessage::get_current_preset_num(){
    //ByteVector SparkMessage::get_current_preset_num(){
       std::vector<ByteVector> msg;      
       ByteVector msg_vec = {0x01, 0xfe, 0x00, 0x00, 0x53, 0xfe, 
                             0x17, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
                             0xf0, 0x01, 0x08, 0x00, 0x02, 0x10, 0xf7, 0x79};
      msg.push_back(msg_vec);
      return msg;
    }

    std::vector<ByteVector> SparkMessage::get_current_preset(){
    //ByteVector SparkMessage::get_current_preset(){

      std::vector<ByteVector> msg;
      ByteVector msg_vec = {0x01, 0xfe, 0x00, 0x00, 0x53, 0xfe, 
                            0x3c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
                            0xf0, 0x01, 0x0a, 0x01, 0x02, 0x01, 0x00, 0x01, 0x00, 0x00, 
                            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
                            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
                            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
                            0x00, 0x00, 0x00, 0xf7};

      msg.push_back(msg_vec);
      return msg;
    }

    std::vector<ByteVector> SparkMessage::change_effect_parameter (std::string pedal, int param, float val){
    //ByteVector SparkMessage::change_effect_parameter (std::string pedal, int param, float val){
      cmd = '\x01'; 
      sub_cmd = '\x04';
  
      start_message (cmd, sub_cmd);
      add_prefixed_string (pedal);
      add_byte((byte)param);
      add_float(val);
      return end_message();  
      
    }

    std::vector<ByteVector> SparkMessage::change_effect (std::string pedal1, std::string pedal2){
    //ByteVector SparkMessage::change_effect (std::string pedal1, std::string pedal2){
      cmd = '\x01'; 
      sub_cmd = '\x06';
  
      start_message (cmd, sub_cmd);
      add_prefixed_string (pedal1);
      add_prefixed_string (pedal2);
      return end_message();    
      
      
    }
    
    std::vector<ByteVector> SparkMessage::change_hardware_preset (int preset_num){
    //ByteVector SparkMessage::change_hardware_preset (int preset_num){
      cmd = '\x01'; 
      sub_cmd = '\x38';
  
      start_message (cmd, sub_cmd);
      add_byte((byte)0);
      add_byte((byte)preset_num-1);
      return end_message();  
  
    }
    
    std::vector<ByteVector> SparkMessage::turn_effect_onoff (std::string pedal, boolean enable){
    //ByteVector SparkMessage::turn_effect_onoff (std::string pedal, boolean enable){
      cmd = '\x01'; 
      sub_cmd = '\x15';

      start_message (cmd, sub_cmd);
      add_prefixed_string (pedal);
      add_onoff(enable);
      Serial.printf("Switching pedal %s to %s\n", pedal.c_str(), enable ? "On" : "Off");
      return end_message();   
    }
    
    std::vector<ByteVector> SparkMessage::create_preset (preset preset_data){
    //ByteVector SparkMessage::create_preset (preset preset_data){
      cmd = '\x01'; 
      sub_cmd = '\x01';

      int this_chunk = 0;
  
      start_message (cmd, sub_cmd);
      add_byte('\x00');
      add_byte('\x7f');       
      add_long_string (preset_data.uuid);
      add_string (preset_data.name);
      add_string (preset_data.version);
      std::string descr = preset_data.description;
      if (descr.size() > 31){
          add_long_string (descr);
      }
      else{
          add_string (descr);
      }
      add_string (preset_data.icon);
      add_float (preset_data.bpm);
      add_byte ((byte)(0x90 + 7));        // always 7 pedals
      for (int i=0; i<7; i++){
        pedal curr_pedal = preset_data.pedals[i];
        add_string (curr_pedal.name);
        add_onoff(curr_pedal.isOn);
        std::vector<parameter> curr_pedal_params = curr_pedal.parameters;
        int num_p = curr_pedal_params.size();
        add_byte ((byte)(num_p + 0x90));
        for (int p=0; p<num_p; p++){
              add_byte((byte)p); 
              add_byte((byte)'\x91');
              add_float (curr_pedal_params[p].value);
        }
      }
      add_byte ((byte)(preset_data.filler));                   
      return end_message ();
  }

    std::vector<ByteVector> SparkMessage::send_ack(byte seq, byte cmd) {
    	std::vector<ByteVector> ack_cmd;
    	ByteVector ack = { 0x01, 0xfe, 0x00, 0x00, 0x41, 0xff, 0x17, 0x00, 0x00,
    			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xf0, 0x01 };
    	ack.push_back(seq);
    	ack.push_back(0x00);
    	ack.push_back(0x04);
    	ack.push_back(cmd);
    	ack.push_back(0xf7);

    	ack_cmd.push_back(ack);
    	return ack_cmd;
    }

