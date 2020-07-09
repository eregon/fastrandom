require 'securerandom'
require_relative '../../ext/cext/random_cext.so'

module FastCExtRandom
  class Random
    def initialize(seed = nil)
      raise ArgumentError, "Seeding not supported (yet)!" unless seed.nil?

      u64_array = ::SecureRandom.random_bytes(16).unpack('QQ')
      init(*u64_array)
    end
  end
end
