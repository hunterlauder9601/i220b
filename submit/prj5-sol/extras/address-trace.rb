#!/usr/bin/ruby

#Script to generate address traces:
#
#   random:  generate a random address trace.
#
#   matrix:  generate a address trace for accessing a matrix by
#            row or col
#
#   program: generate a typical program address trace with random
#            intra-function jumps and random inter-function
#            call/return.

COURSE = 'cs220'
COURSE_GEM_PATH = "#{ENV['HOME']}/#{COURSE}/lib/gems"
Gem.paths = { 'GEM_PATH' => "#{COURSE_GEM_PATH}:#{Gem.user_dir}:#{Gem.dir}" }

require 'main'

class MatrixTracer

  ELEMENT_SIZE = 8

  def initialize(params)
    @base, @size, @access, mem = 
      %w(base size access mem).map { |k| params[k].value }
    if @base + ELEMENT_SIZE * @size**2 >= mem
      abort "memory size of #{mem} too small for matrix of " +
        "size #{@size}x#{@size} stored at #{@base}" 
    end
  end

  def gen(len)
    n = 0
    while true do
      (0...@size).each do |i|
        (0...@size).each do |j|
          index = (@access == 'row') ? @size*i + j : @size*j + i
          yield @base + index*ELEMENT_SIZE
          n += 1
          return if n >= len
        end
      end        
    end
  end

end

class RandomTracer

  def initialize(params)
    @base, @mem, seed = %w(base mem rand).map { |k| params[k].value }
    srand seed
  end

  def gen(len)
    n = 0
    range = @mem - @base
    while true
      yield @base + rand(range)
      n += 1
      return if n >= len
    end
  end

end

class ProgramTracer

  MAX_NEAR_RANGE = 1024 * 10
  MAX_FAR_RANGE = 1024**3
  MAX_OP_LEN = 15

  def initialize(params)
    @base, @mem, @call_prob, @jump_prob, seed = 
      %w(base mem call-prob jump-prob rand).map { |k| params[k].value }
    srand seed
    size = @mem - @base
    @near_range = size / 0x1000
    @near_range = MAX_NEAR_RANGE if @near_range > MAX_NEAR_RANGE
    @log_near = Math.log10(@near_range).to_i
    @far_range = size / 0x100
    @far_range = MAX_FAR_RANGE if @far_range > MAX_FAR_RANGE
    @stack = [ ]
  end

  def next_addr(addr)
    toss = rand
    if toss < @jump_prob 
      #use log's to make close jumps more likely than far jumps
      r = rand(2..@log_near)
      offset = rand((10**(r - 1))..(10**r - 1))
      offset = @near_range if offset > @near_range
      jmp_addr = (addr + (rand < 0.5 ? -1 : +1) * offset).to_i
      #printf "jump %08x\n", jmp_addr
      return jmp_addr
    end
    toss = rand
    if toss < @call_prob
      if (@stack.size == 0 || rand < 0.5) 
        @stack.push((addr + 1 + rand(MAX_OP_LEN)).to_i)
        call_addr = (addr + (rand < 0.5 ? -1 : +1) * rand(@far_range)).to_i
        #printf "call %08x\n", call_addr
        return call_addr
      else
        ret_addr = @stack.pop
        #printf "ret %08x\n", ret_addr
        return ret_addr
      end
    end
    return addr + 1 + rand(MAX_OP_LEN)
  end

  def gen(len)
    n = 0
    addr = @base + (@mem - @base)/2
    while true
      addr = next_addr addr
      addr = @base if addr < @base || addr > @mem
      yield addr
      n += 1
      return if n >= len
    end
  end

end

LENGTH_UNITS = 1000

Main do

  option('length', 'l') {
    argument_required
    cast :int
    default 100
    description "trace length in units of #{LENGTH_UNITS}"
  }

  option('base', 'b') {
    argument_required
    cast :int
    default 0x1000
    description 'low base address'
  }

  option('mem', 'm') {
    argument_required
    cast do |v| 
      if match = v.match(/^(\d+)([kmg])$/)
        m, u = match.captures
        m.to_i * 1024 ** ('kmg'.index(u) + 1)
      else
        v.to_i
      end
    end
    default (512 * 1024**2).to_s
    description 'total memory size; optional suffix k, m, g'
  }

  option('rand', 'r') {
    argument_required
    cast :int
    default 0
    description 'seed for random number generator'
  }

  mode 'random' do
    description 'random address trace'
    def run
      list RandomTracer.new(params)
    end
  end

  mode 'matrix' do
    description 'address trace for row or column access to matrix'
    option('size', 's') {
      argument_required
      default 1024
      cast :int
      description '# of rows and cols'
    }
    option('access', 'a') {
      argument_required
      default 'row'
      validate { |v| v == 'row' || v == 'col' }
      description 'access by row|col'
    }

    def run
      list MatrixTracer.new(params)
    end

  end

  mode 'program' do
    description <<-desc
      address trace for typical program with local and global branches
    desc
    option('jump-prob', 'j') {
      argument_required
      default 0.2
      cast :float
      description 'probability of taking intra-function jump'
    }
    option('call-prob', 'c') {
      argument_required
      default 0.1
      cast :float
      description 'probability of call/return'
    }

    def run
      list ProgramTracer.new(params)
    end


  end

  def run
    print usage
  end

  def list(obj)
    obj.gen(params['length'].value * LENGTH_UNITS) do |a|
      printf "%08x\n", a
    end
  end


end
