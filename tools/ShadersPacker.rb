require 'fileutils'

class ShadersPacker

  GL_FRAGMENT_SHADER = 0x8B30
  GL_VERTEX_SHADER   = 0x8B31

  module ClassMethods
    def shader_name(name)
      "shaders/#{name}.vert\0shaders/#{name}.frag\0"
    end

    def shader_names(names)
      namelist = ""
      names.each {|name| namelist << shader_name(name) if name}
      namelist
    end

    @@typenames = {GL_VERTEX_SHADER: "vert", GL_FRAGMENT_SHADER: "frag"}
    def index_of(namelist, name, type)
      namelist.index("shaders/#{name}.#{@@typenames[type]}")
    end

    def variable_location(declaration_line)
      location =
        declaration_line.scan(/\(\s*location\s*=\s*(\d+)\s*\)/).
          last.last
      if location then location.to_i else nil end
    end
    
    
    def add_identifier_if_line_contain(pattern:, names:, line:)
      if (line =~ pattern)
        # location = variable_location(line)
        identifier_name = line.scan(/\b\w+\b/).last
        names << identifier_name
      end
    end
    
    def uniforms_of(shaders_path)
      uniforms_names = []
      shaders_path.each {|filepath|
        File.open(filepath, "r") {|f|
          f.each_line {|shader_line|
            # This can get fooled by comments like : // uniform
            add_identifier_if_line_contain(
              pattern: /\buniform\b/,
              names: uniforms_names,
              line: shader_line
            )
          }
        }
      }
      uniforms_names
    end

    def attributes_of(shaders_path)
      attributes_names = []
      File.open(shaders_path[0]) do |f|
        f.each_line do |shader_line|
           add_identifier_if_line_contain(
             pattern: /\battribute\b/,
             names: attributes_names,
             line: shader_line
           )
        end
      end
      attributes_names
    end

    # Clear needs to be factorised
    def shaders_enums(shaders_infos_hash, strings_lengths)
      block_end = ->(enum_name, enum_string) {
        enum_string << "\tn_#{enum_name}s,\n};\n\n"
      }
      glsl_shader_name_enum  = "enum glsl_files {\n"
      glsl_program_name_enum = "enum glsl_program_name {\n"
      glsl_program_uniform_enum  = "enum glsl_program_uniform {\n"

      programs_elements_enums = []

      shaders_infos_hash.each do |shader_name, elements|

        glsl_shader_name_enum <<
          "\tglsl_file_#{shader_name}_vsh,\n\tglsl_file_#{shader_name}_fsh,\n"
        glsl_program_name_enum <<
          "\tglsl_program_#{shader_name},\n"

        program_attributes_enum = "enum #{shader_name}_attribute {\n"
        (0...elements[:attributes].length).each do |i|
          shader_attribute_name = elements[:attributes][i]
          program_attributes_enum <<
            "\t#{shader_name}_shader_attr_#{shader_attribute_name},\n"
        end
        block_end["#{shader_name}_attribute", program_attributes_enum]

        (0...elements[:uniforms].length).each do |i|
          shader_uniform_name = elements[:uniforms][i]
          glsl_program_uniform_enum <<
            "\t#{shader_name}_shader_unif_#{shader_uniform_name},\n"
        end

        programs_elements_enums << program_attributes_enum
      end

      block_end["glsl_file", glsl_shader_name_enum]
      block_end["glsl_program", glsl_program_name_enum]
      block_end["glsl_program_uniform", glsl_program_uniform_enum]

      c_enums = ""
      c_enums << glsl_shader_name_enum
      c_enums << glsl_program_name_enum
      c_enums << glsl_program_uniform_enum
      programs_elements_enums.each do |program_specific_enum|
        c_enums << program_specific_enum
      end
      
      struct_def = <<"STRUCTEND"
struct glsl_elements {
	struct { uint16_t n, pos; } attributes, uniforms;
};

struct glsl_shader {
	GLuint type;
	uint32_t str_pos;
};

struct glsl_programs_shared_data {
	GLuint programs[n_glsl_programs];
	GLint  unifs[n_glsl_program_uniforms];
	struct glsl_shader shaders[n_glsl_files];
	struct glsl_elements metadata[n_glsl_programs];
	uint8_t strings[#{strings_lengths[:names]}];
	uint8_t identifiers[#{strings_lengths[:identifiers]}];
};


STRUCTEND
      
      c_header = "#ifndef MYY_GENERATED_OPENGL_ENUMS_H\n"
      c_header << "#define MYY_GENERATED_OPENGL_ENUMS_H 1\n"
      c_header << "#include <myy/current/opengl.h>\n"
      c_header << "#include <stdint.h>\n"
      c_header << (c_enums + struct_def)
      c_header << "#endif\n"
    end
    
    def shaders_elements(names, namelist)
      shaders_paths = namelist.split("\0").each_slice(2).to_a
      shaders_elements = {}
      names.each_with_index {|shader_name, i|
        shaders_elements[shader_name] = {
          attributes: attributes_of(shaders_paths[i]),
          uniforms: uniforms_of(shaders_paths[i]),
        }
      }
      shaders_elements
    end

    def shaders_elements_list(shaders_elements)
      list = ""
      positions = []

      shaders_elements.each do |name, elements_types|
        positions << elements_types[:attributes].length
        positions << list.length
        list << elements_types[:attributes].join("\0")
        list << "\0"
        positions << elements_types[:uniforms].length
        positions << list.length
        list << elements_types[:uniforms].join("\0")
        list << "\0"
      end
      
      return list, positions
    end
    
    def total_uniforms_in(shaders_elements)
      
      shaders_elements.values.inject(0) do |total, elements|
        total + elements[:uniforms].length
      end
                                        
    end
    
    def pack(names, out_filepath, shaders_root_path)

      count = names.length
      programs = Array.new(count, 0)
      namelist = shader_names(names)
      elements = shaders_elements(names, namelist)
      uniforms_locations = 
          Array.new(total_uniforms_in(elements), 0)
      identifiers_list, identifiers_positions =
          shaders_elements_list(elements)
      types = []
      names.each {|name|
        types << GL_VERTEX_SHADER
        types << index_of(namelist, name, :GL_VERTEX_SHADER)
        types << GL_FRAGMENT_SHADER
        types << index_of(namelist, name, :GL_FRAGMENT_SHADER)
      }
      content = programs.pack("I<*")
      content << uniforms_locations.pack("I<*")
      content << types.pack("I<*")
      content << identifiers_positions.pack("S<*")
      content << namelist.bytes.pack("C*")
      content << identifiers_list.bytes.pack("C*")
      
      c_helper_header = 
        File.realpath("src/generated/opengl/shaders_infos.h")

      File.write(out_filepath, content)
      puts "Writing the C helper header at : #{c_helper_header}"
      File.write(
        c_helper_header,
        shaders_enums(
          elements, 
          {names: namelist.length,
           identifiers: identifiers_list.length}
        )
      )
      
      
    end

  end

  extend ClassMethods

end

if ARGV.length > 2
  abort("ShadersPackers.rb /path/to/shaders /path/to/output.pack")
end

$shaders_directory =
  (if ARGV[0] then File.realpath(ARGV[0]) else File.realpath("shaders") end)
$output_filepath =
  (if ARGV[1] then File.realpath(ARGV[1]) else File.realpath("data/shaders.pack") end)

require 'set'

puts "Provided shaders directory : #{$shaders_directory}"
puts "Write output at : #{$output_filepath}"

def get_shaders_names_automatically(shaders_dir)
  shader_names = Set.new
  Dir["#{shaders_dir}/*.{frag,vert}"].each do |shader_filename|
    shader_names.add(File.basename(shader_filename, ".*"))
  end
  shader_names.to_a
end

ShadersPacker.pack(
  get_shaders_names_automatically($shaders_directory),
  $output_filepath,
  $shaders_directory
)

puts "Finished !"
