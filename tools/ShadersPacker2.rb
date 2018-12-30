require 'fileutils'

def align_on_pow2(value, pow2)
	pmask = pow2 - 1
	return (value + (pmask)) & (~pmask)
end

def pad_on_pow2_with_zero(buffer, pow2)
	buffer << "\u0000"
	pow2_size = align_on_pow2(buffer.length, pow2)

	(pow2_size - buffer.length).times do
		buffer << "\u0000"
	end
	buffer
end


def add_identifier_if_line_contain(pattern:, names:, line:)

	line_matched = (line =~ pattern)
	if (line_matched)
		identifier_name = line.scan(/\b\w+\b/).last
		names << identifier_name
		true
	else
		false
	end
end

def add_attribute_if_attribute_line(
	attributes:, line:, remaining_ids:)
	return false if remaining_ids.empty?

	names = []
	attribute_added = add_identifier_if_line_contain(
		pattern: /\battribute\b/,
		names: names,
		line: line)

	if attribute_added
		# Get the attribute location
		# On GLES 2.0, we just put the "location =" in comments
		# Using "5" as the provided location
		# On success, we will get ["5"]
		# On failure, we will get nil
		location_id_string = line.scan(/location\s*=\s*(\d+)/).last
		potential_id = -1
		if (location_id_string)
			potential_id = location_id_string.last.to_i
		end

		id_index = remaining_ids.index(potential_id)
		if (remaining_ids.include?(potential_id))
			location = potential_id
			remaining_ids.delete(potential_id)
		else
			location = remaining_ids.shift
		end

		attributes << {name: names.last, id: location}
	end
end

def parse_shader_attribs_unifs(data, attributes, uniforms)

	valid_attribs_id = (0...16).to_a
	data.each_line do
		|shader_line|
		add_attribute_if_attribute_line(
			attributes: attributes,
			line: shader_line,
			remaining_ids: valid_attribs_id)
		add_identifier_if_line_contain(
			pattern: /\buniform\b/,
			names: uniforms,
			line: shader_line)
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
		else
			status[:section] = :not_in_a_section
			puts "#{title} is invalid\nBack to no section"
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

GL_VERTEX_SHADER   = 0x8B31
GL_FRAGMENT_SHADER = 0x8B30
SHADER_FILE_TYPES = {
	:"vert" => GL_VERTEX_SHADER,
	:"vsh"  => GL_VERTEX_SHADER,
	:"frag" => GL_FRAGMENT_SHADER,
	:"fsh"  => GL_FRAGMENT_SHADER
}

def shader_file_type(shader_filename)
	extension = shader_filename.split('.').last.to_sym
	SHADER_FILE_TYPES[extension]
end

MetadataHeader = Struct.new(:signature, :data_offset, :metadata_offset, :n_programs, :programs)
Program = Struct.new(:name_offset, :metadata_offset)

ProgramMetadata = Struct.new(:section_size, :id_offset_in_struct)
ShadersMetadata = Struct.new(:section_size, :n_shaders, :shaders)
Shader = Struct.new(:type, :data_size, :data_offset)

AttributesMetadata = Struct.new(:section_size, :n_attributes, :attributes)
Attribute = Struct.new(:name_offset, :bind_id)

UniformsMetadata = Struct.new(:section_size, :n_uniforms, :uniforms)
Uniform = Struct.new(:offset_in_struct, :name_pos)

MYYP_SIGNATURE = 0x5059594D

$header        = MetadataHeader.new(MYYP_SIGNATURE,0,0,0, [])
$global_offset = 0
$data_section  = ""
$packed_mdata  = ""
$programs      = []

$big_program_structure = "struct {\n"
$c_header = ""

def generate_cfile(filepath)
	$big_program_structure << "\n} __attribute__((packed, aligned(8))) myy_programs;\n"
	$c_header << "\n"
	$c_header << $big_program_structure

	puts $c_header
	File.write(filepath, $c_header)
end

def generate_metadata(filepath)
	header = [
		$header.signature,
		$header.data_offset,
		$header.metadata_offset,
		$header.n_programs
	]
	packed_programs_mdata = ""
	$programs.each do |program|
		packed_programs_mdata << [
			program.name_offset, program.metadata_offset
		].pack("I<*")
	end

	dummy_packed_header = header.pack("I<*")
	offset = 0
	offset += dummy_packed_header.length
	offset += packed_programs_mdata.length
	$header.metadata_offset = offset
	offset += $packed_mdata.length
	$header.data_offset = offset

	header = [
		$header.signature,
		$header.data_offset, $header.metadata_offset,
		$header.n_programs
	]
	whole_file = header.pack("I<*")
	whole_file << packed_programs_mdata
	whole_file << $packed_mdata
	whole_file << $data_section

	File.write(filepath, whole_file)
end

def parse_program_line(status, line)
	if (/^(?<program_name>\w+):/ =~ line.strip)

		puts "\tProgram name : #{program_name}"
		program_name_delimiter = ':'
		the_rest_start_at = line.index(program_name_delimiter)+1
		shader_filenames = line[the_rest_start_at..-1].split(',').map(&:strip).map(&:downcase)
		puts "\t#{line[the_rest_start_at..-1]}"
		puts "\tShader filenames : #{shader_filenames}"

		# Failure will just make the whole program terminates.
		# If the program terminates,
		# the metadata won't be written on disk.
		# So we can assume that everything will be fine.
		$programs << Program.new($data_section.length, $packed_mdata.length)
		$header.n_programs += 1
		$data_section << program_name
		$data_section << "\u0000"

		shaders_mdata    = ShadersMetadata.new(0,0,[])
		attributes_mdata = AttributesMetadata.new(0,0,[])
		uniforms_mdata   = UniformsMetadata.new(0,0,[])

		shaders    = []
		attributes = []
		uniforms   = []

		uniforms_offset = $global_offset

		program_id_offset = uniforms_offset
		attributes_enum = "enum myy_shader_#{program_name}_attribs {\n"

		program_structure  = "\tstruct {\n"
		program_structure << "\t\tGLuint id;\n"
		uniforms_struct    = "\t\tstruct {\n"
		uniforms_offset += 4

		shader_filenames.each do
			|shader_filename|
			shaders_mdata.n_shaders += 1

			puts "Parsing #{shader_filename}"

			shader_attributes = []
			shader_uniforms   = []
			
			shader_type = shader_file_type(shader_filename)
			next if shader_type.nil?

			data = (File.read(shader_filename) << "\0")
			shaders << Shader.new(shader_type, data.length, $data_section.length)
			$data_section << data

			parse_shader_attribs_unifs(data, shader_attributes, shader_uniforms)

			shader_attributes.each do
				|attribute|
				name = attribute[:name]
				id   = attribute[:id]
				attributes_enum << "\t#{program_name}_#{name} = #{id},\n"

				p attribute
				p id
				attributes << Attribute.new($data_section.length, id)
				$data_section << name
				$data_section << "\u0000"
				attributes_mdata.n_attributes += 1
			end

			shader_uniforms.each do
				|uniform|
				uniforms_struct << "\t\t\tGLint #{uniform};\n"

				uniforms << Uniform.new(uniforms_offset, $data_section.length)
				uniforms_offset += 4
				p uniform
				$data_section << uniform
				$data_section << "\u0000"
				uniforms_mdata.n_uniforms += 1
			end
		end

		# Writing 
		uniforms_struct << "\t\t} uniform;\n"
		program_structure << uniforms_struct
		program_structure << "\t} #{program_name};\n"

		attributes_enum << "};\n"

		$big_program_structure << program_structure
		$c_header << attributes_enum

		# Writing the metadata
		# There's clearly a pattern
		# I'm too tired and busy to factorize this

		dummy_packed_program_section_header =
			[0, program_id_offset].pack("I<*")
		packed_program_section =
			[dummy_packed_program_section_header.length, program_id_offset].pack("I<*")
		
		packed_shaders_section = ""
		packed_shaders_mdata = ""

		shaders.each do
			|shader|
			packed_shaders_mdata <<
				[shader[:type], shader[:data_size], shader[:data_offset]].pack("I<*")
		end

		dummy_shaders_section_header =
			[shaders_mdata.section_size, shaders_mdata.n_shaders].pack("I<*")
		shaders_mdata.section_size =
			packed_shaders_mdata.length + dummy_shaders_section_header.length
		packed_shaders_section << [shaders_mdata.section_size, shaders_mdata.n_shaders].pack("I<*")
		packed_shaders_section << packed_shaders_mdata


		packed_attributes_section = ""
		packed_attributes_mdata = ""
		attributes.each do
			|attribute|
			p [attribute[:name_offset], attribute[:bind_id]]
			packed_attributes_mdata << [attribute[:name_offset], attribute[:bind_id]].pack("I<*")
		end
		dummy_attributes_section_header =
			[attributes_mdata.section_size, attributes_mdata.n_attributes].pack("I<*")
		attributes_mdata.section_size = 
			packed_attributes_mdata.length + dummy_attributes_section_header.length
		packed_attributes_section << 
			[attributes_mdata.section_size, attributes_mdata.n_attributes].pack("I<*")
		packed_attributes_section << packed_attributes_mdata

		packed_uniforms_section = ""
		packed_uniforms_mdata = ""
		uniforms.each do
			|uniform|
			packed_uniforms_mdata << [uniform[:offset_in_struct], uniform[:name_pos]].pack("I<*")
		end
		dummy_uniforms_section_header = 
			[uniforms_mdata.section_size, uniforms_mdata.n_uniforms].pack("I<*")
		uniforms_mdata.section_size = packed_uniforms_mdata.length + dummy_uniforms_section_header.length
		packed_uniforms_section << [
			uniforms_mdata.section_size, uniforms_mdata.n_uniforms
		].pack("I<*")
		packed_uniforms_section << packed_uniforms_mdata

		$packed_mdata << packed_program_section
		$packed_mdata << packed_shaders_section
		$packed_mdata << packed_attributes_section
		$packed_mdata << packed_uniforms_section

		$global_offset = uniforms_offset
	end
end

def parse_texture_line(status, line)
	if (/^(?<texture_name>\w+):\s*(?<texture_file>\S+)$/ =~ line.strip)
		status[:definitions][:textures][:data] << "	GLuint #{texture_name};"
	end
end

def parse_buffer_line(status, line)
end

SECTION_METHODS = {
	not_in_a_section:  method(:parse_useless_line),
	programs:          method(:parse_program_line),
	textures:          method(:parse_texture_line),
	buffers:           method(:parse_buffer_line),
	
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

generate_metadata("shaders.pack")
generate_cfile("shaders.h")
