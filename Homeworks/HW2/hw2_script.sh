#!/bin/bash

script_dir=$(dirname "$(realpath "$0")")
: ${USER:=$(whoami)}
test_parameters=(100 200 500)
answer=(11208656 2317752448 2589282704)
cpp_files=$(find "$script_dir" -maxdepth 1 -name "*.cpp")

for cpp_file in $cpp_files; do
    filename=$(basename "$cpp_file" .cpp)
    echo "=================================  $filename  ========================================"
    echo "" > "${script_dir}/${filename}_result.txt"
    g++ "$cpp_file" -o "${script_dir}/${filename}.out"
    
    # compile scuuess
    if [ $? -eq 0 ]; then
        echo "Compile success: ${cpp_file} -> ${filename}.out"
        error_count=0 

        for i in "${!test_parameters[@]}"; do
            current_param=${test_parameters[$i]}
            expected_checksum=${answer[$i]}
            wrong_answer=0
            echo "      Testing with parameter = $current_param"

            result=$(echo -e "$current_param\n" | "${script_dir}/${filename}.out")

            # check your checksum
            checksums=$(echo "$result" | grep "Checksum")
            while read -r line; do
                checksum=$(echo "$line" | awk -F'Checksum: ' '{print $2}')
                if [ "$checksum" -ne "$expected_checksum" ]; then
                    wrong_answer=1
                fi
            done <<< "$checksums"
            if [ "$wrong_answer" -eq 1 ]; then
                error_count=$((error_count + 1))
            fi
            
            # check elapsed time
            elapsed_times=$(echo "$result" | grep "time")
            elapsed_time_array=()
            while read -r line; do
                elapsed_time=$(echo "$line" | awk -F'time:' '{print $2}'| awk -F'Checksum' '{print $1}') 
                elapsed_time_array+=("$elapsed_time")
            done <<< "$elapsed_times"

            # 將 elapsed time array 寫入結果檔案
            elapsed_time_str=$(echo "${elapsed_time_array[*]}") 
            echo "param: $current_param," >> "${script_dir}/${filename}_result.txt"
            echo "      elapsed time array: $elapsed_time_str" >> "${script_dir}/${filename}_result.txt"
            echo " " >> "${script_dir}/${filename}_result.txt" 

        done

        # total wrong answer number
        echo "Total incorrect checksums for $filename: $error_count"
        echo "Total incorrect checksums for $filename: $error_count" >> "${script_dir}/${filename}_result.txt"

        # check shm deletion
        shared_memory=$(ipcs -m | grep "$USER")
        if [ -n "$shared_memory" ]; then
            echo "Unreleased shared memory exists!"
            echo "Unreleased shared memory exists!" >> "${script_dir}/${filename}_result.txt"
            echo "$shared_memory" | awk '{print $2}' | xargs -I {} ipcrm -m {}
        else
            echo "No unreleased shared memory." 
            echo "No unreleased shared memory." >> "${script_dir}/${filename}_result.txt"
        fi

    else
        echo "Compile failed: ${cpp_file}"
        echo "Compile failed: ${cpp_file}" >> "${script_dir}/${filename}_result.txt"
    fi
done


