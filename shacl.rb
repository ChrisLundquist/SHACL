# $:.unshift(File.dirname(__FILE__) + '/../ext')

require 'barracuda'
MAX_SEARCH_SIZE = 32000
include Barracuda

f = File.new("shacl.cl","r")
prog = Program.new(f.read)
f.close

while(1) do
  message = "foo"
  message_buffer = Buffer.new(message.length)
  message_buffer.each_with_index { |v,i| message_buffer[i] = message[i] }

  output = Buffer.new(MAX_SEARCH_SIZE).to_type(:uchar)
  target_hash = Buffer.new(5).to_type(:int)

  prog.SHAKernel(message_buffer,message_buffer.length,target_hash, output)
 
  output.each_slice(5 * 4) do |a|
    p a.map { |i| i.to_s(16).upcase }.join
  end
end
