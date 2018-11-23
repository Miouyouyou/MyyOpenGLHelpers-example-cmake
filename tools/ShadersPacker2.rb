require 'fileutils'

def add_identifier_if_line_contain(pattern:, names:, line:)
	if (line =~ pattern)
	# location = variable_location(line)
	identifier_name = line.scan(/\b\w+\b/).last
	names << identifier_name
	end
end

def parse_shader_attribs_unifs(shader_filepath, attributes, uniforms)
	puts shader_filepath
	File.open(shader_filepath, "r") do |f|
		f.each_line do |shader_line|
			add_identifier_if_line_contain(
				pattern: /\battribute\b/,
				names: attributes,
				line: shader_line)
			add_identifier_if_line_contain(
				pattern: /\buniform\b/,
				names: uniforms,
				line: shader_line)
		end
	end
end

SECTIONS = [
	:not_in_a_section,
	:programs,
	:textures, 
	:buffers]

def section_change(status, line)
	valid_section = false
	if (/^\s*\[(?<title>[^\]]+)\]\s*/ =~ line)
		section = title.strip.downcase.to_sym
		puts ("section : #{section}")
		valid_section = SECTIONS.include?(section)
		if valid_section
			status[:section] = section
			puts "Parsing section #{section}"
		end
	end
	valid_section
end

status = {
	section: :not_in_a_section,
	definitions: {
		programs: {
			data: "",
			metadata: ""
		},
		buffers: "",
		textures: {
			data: "",
			metadata: ""
		},
	}
}

def parse_useless_line(status, line)
end

SHADER_FILE_TYPES = {
	:"vert" => :GL_VERTEX_SHADER,
	:"vsh"  => :GL_VERTEX_SHADER,
	:"frag" => :GL_FRAGMENT_SHADER,
	:"fsh"  => :GL_FRAGMENT_SHADER
}

def shader_file_type(shader_filename)
	extension = shader_filename.split('.').last.to_sym
	puts extension
	SHADER_FILE_TYPES[extension]
end

def parse_program_line(status, line)
	if (/^(?<program_name>\w+):/ =~ line.strip)

		puts "\tProgram name : #{program_name}"
		program_name_delimiter = ':'
		the_rest_start_at = line.index(program_name_delimiter)+1
		shader_filenames = line[the_rest_start_at..-1].split(',').map(&:strip).map(&:downcase)
		puts "\t#{line[the_rest_start_at..-1]}"
		puts "\tShader filenames : #{shader_filenames}"

		program_uniforms = "	struct {\n"
		program_attributes = "enum myy_shader_#{program_name}_attribs {\n"
		program_metadata_uniforms = "	struct {\n"
		program_metadata_attributes = "	struct {\n"
		program_attributes_names = []
		program_uniforms_names   = []
		program_metadata_files_definition = ""
		shader_filenames.each do
			|shader_filename|
			puts "Parsing #{shader_filename}"
			shader_type = shader_file_type(shader_filename)
			next if shader_type.nil?
			program_metadata_files_definition << "		{\n"
			program_metadata_files_definition << "			.type = #{shader_type},\n"
			program_metadata_files_definition << "			.name = #{shader_filename}\n"
			program_metadata_files_definition << "		},\n"
			
			parse_shader_attribs_unifs(shader_filename,
				program_uniforms_names, program_attributes_names)
		end

		program_attributes_names.each do |attrib_name|
			program_attributes << "		#{program_name}_attrib_#{attrib_name},\n"
		end
		program_uniforms_names.each do |uniform_name|
			program_uniforms << "		GLint #{uniform_name};\n"
		end
		program_attributes          << "};\n"
		program_uniforms            << "	} uniform;\n"
		program_metadata_uniforms   << "} uniform;\n"
		program_metadata_attributes << "} attributes;\n"

		program_structure  = "struct {\n"
		program_structure << "	GLuint id;\n"
		program_structure << program_uniforms
		program_structure << "} myy_sp_#{program_name};\n"

		program_metadata_structure  = "struct {\n"
		program_metadata_structure << "	struct gl_shader_file shader_files[#{shader_filenames.length}];\n"
		program_metadata_structure << "	GLchar const * __restrict const attribute_names[#{program_attributes_names.length}];\n"
		program_metadata_structure << "	GLchar const * __restrict const uniforms_names[#{program_uniforms_names.length}];\n"
		program_metadata_structure << "} #{program_name};"

		program_metadata_structure_definition =  ".#{program_name} = {\n"
		program_metadata_structure_definition << "	.shader_files = {\n"
		program_metadata_structure_definition << program_metadata_files_definition
		program_metadata_structure_definition << "	},\n"
		program_metadata_structure_definition << "	.attributes = {\n"
		program_metadata_structure_definition << "		#{program_attributes_names.map(&:inspect).join(",\n\t\t")}\n"
		program_metadata_structure_definition << "	},\n"
		program_metadata_structure_definition << "	.uniforms = {\n"
		program_metadata_structure_definition << "		#{program_uniforms_names.map(&:inspect).join(",\n\t\t")}\n"
		program_metadata_structure_definition << "	}\n"
		program_metadata_structure_definition << "}\n"

		puts "program_attributes :\n#{program_attributes}"
		puts "program_structure  :\n#{program_structure}"
		puts "program_metadata_structure :\n#{program_metadata_structure}"
		puts "program_metadata_definition :\n#{program_metadata_structure_definition}"
	end
end

def parse_texture_line(status, line)
	if (/^(?<texture_name>\w+):\s*(?<texture_file>\S+)$/ ~= line.strip) then
	status[:definitions][:textures][:data] << "	GLuint #{texture_name};"
end

def parse_buffer_line(status, line)
end

SECTION_METHODS = {
	not_in_a_section:  method(:parse_useless_line),
	programs:          method(:parse_program_line),
	textures:          method(:parse_texture_line),
	buffers:           method(:parse_buffer_line)
}

abort("./program.rb /path/to/metadata.ini") if (ARGV.length != 1)

config_file_path = ARGV[0]

abort("#{config_file_path} does not exist") unless File.exist?(config_file_path)

Dir.chdir(File.dirname(config_file_path))

File.open(File.basename(config_file_path)) do |f|
	f.each_line do |line|
		next if section_change(status, line)
		SECTION_METHODS[status[:section]][status, line]
	end
end
