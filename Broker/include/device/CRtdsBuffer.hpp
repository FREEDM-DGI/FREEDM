class CRtdsBuffer
{
public:
    /// allows reading from the buffer
    const SettingValue& operator[](const Signal sig) const;

    /// allows writing to the buffer
    SettingValue& operator[](const Signal sig);

private:
    /// the actual buffer, sent to or received from the FPGA
    std::vector<SettingValue> m_buffer;    

    /// provides synchronization for the buffer
    boost::shared_mutex m_mutex;

    /// translates a device signal into a buffer index
    std::map<Signal, size_t> m_signalToIndexMap;
};
